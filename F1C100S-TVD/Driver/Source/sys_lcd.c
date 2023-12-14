#include "sys_lcd.h"
#include "reg-debe.h"
#include "reg-tcon.h"
#include "sys_interrupt.h"
#include "sys_uart.h"
#include "sys_gpio.h"
#include "sys_delay.h"
#include "sys_io.h"
#include <stdlib.h>
//-----------------------
#include "sys_lcd_conf.h"
//-----------------------
#if (defined LCD_TYPE_TV_PAL_720_576)||(defined LCD_TYPE_TV_NTSC_720_480)
	#include "sys_tve.h"
#endif

/************************************************************************************
当boot已经初始化了LCD，应用再次初始化LCD时解决显示过度的方法：
1.不初始化，直接使用第一次初始化的显存。
	f1c100s_lcd_set_addr();  //设置lcd地址
  显存地址=f1c100s_debe_get_address_x(0);	 //获取显存地址
	
2.不初始化，直接设置新的显存地址。
  lcd_wait_vertical_blanking_interrupt();//帧中断
	f1c100s_lcd_set_addr();  //设置lcd地址
	delay_ms(1);//跳过本帧-留时间给GUI绘图
  f1c100s_debe_set_address_x(0,显存地址);	 //设置新的显存地址	
	
3.再次初始化-当第一次初始化的IO与第二次初始化的IO不一样时，Sys_LCD_Init函数里需要先关闭时钟。
	Sys_LCD_Init(W,H,显存1,显存2);	
************************************************************************************/
//LCD时序
__align(4) struct __lcd_timing lcd_timing=
{
/*模式 ----水平时序------,     -----垂直时序------,   -极性控制--,     串并行  CPU_MODE 	PCKL=(24MHz*N)/M/F  [5<F<96]*/ 
/*MODE WIDTH,HFP,HBP,HSPW,  HEIGHT,VFP,VBP,VSPW,  HA,VA,DA,CA,      s,  		  0, 		N, M, F,                    */	
   //1,     800, 0, 0,  1,      480, 0, 0,   1,   0, 0, 1, 0,      1,  		  0, 		65, 4,13 //PCLK=30MHZ
	 1,     800, 1, 87,  1,      480, 1, 31,   1,   0, 0, 1, 0,      1,  		  0, 		127, 1,6 //
};	
//全局结构体
struct fb_f1c100s_pdata_t __lcd_pdat;
struct fb_f1c100s_pdata_t * lcd_pdat;

/*开TCON时钟*/
void Tcon_Clk_Open(void)
{
	S_BIT(CCU_BUS_CLK_GATING_REG1,4);
	S_BIT(CCU_TCON_CLK_REG,31);
	S_BIT(CCU_BUS_SOFT_RST_REG1,4);
	
}
/*开Debe时钟*/
void Debe_Clk_Open(void)
{
  S_BIT(CCU_BUS_CLK_GATING_REG1,12);
	S_BIT(CCU_BUS_SOFT_RST_REG1,12);
}

/*------------------------
设置层窗口大小
------------------------*/
void Set_LayerX_window(int Layer,int x,int y,int w,int h)
{
	struct f1c100s_debe_reg_t * debe = (struct f1c100s_debe_reg_t *)lcd_pdat->virtdebe;
	switch(Layer)
	{
		case 0:
			write32((virtual_addr_t)&debe->layer0_pos, (y<< 16) | (x << 0));	
			write32((virtual_addr_t)&debe->layer0_size, ((h - 1) << 16) | ((w - 1) << 0));			
			break;
		case 1:
			write32((virtual_addr_t)&debe->layer1_pos, (y<< 16) | (x << 0));	
			write32((virtual_addr_t)&debe->layer1_size, ((h - 1) << 16) | ((w - 1) << 0));			
			break;
		case 2:
			write32((virtual_addr_t)&debe->layer2_pos, (y<< 16) | (x << 0));	
			write32((virtual_addr_t)&debe->layer2_size, ((h - 1) << 16) | ((w - 1) << 0));			
			break;
		case 3:
			write32((virtual_addr_t)&debe->layer3_pos, (y<< 16) | (x << 0));	
			write32((virtual_addr_t)&debe->layer3_size, ((h - 1) << 16) | ((w - 1) << 0));			
			break;
	}
}
/*------------------------
使能层视频通道
------------------------*/
void Enable_LayerX_Video(int Layer)
{
	struct f1c100s_debe_reg_t * debe = (struct f1c100s_debe_reg_t *)lcd_pdat->virtdebe;
	S_BIT((virtual_addr_t)&debe->layer0_attr0_ctrl,1);	
	/*switch(Layer)
	{
		case 0:
			S_BIT((virtual_addr_t)&debe->layer0_attr0_ctrl,1);	
			break;
		case 1:
			S_BIT((virtual_addr_t)&debe->layer1_attr0_ctrl,1);			
			break;
		case 2:
			S_BIT((virtual_addr_t)&debe->layer2_attr0_ctrl,1);			
			break;
		case 3:
			S_BIT((virtual_addr_t)&debe->layer3_attr0_ctrl,1);				
			break;
	}*/
}

/*------------------------
设置数据格式 
[forma RGB565=0x5,stride=4][forma ARGB8888=0xA,stride=5]
------------------------*/
void Set_Layer_format(int Layer,int format,int stride)
{
	struct f1c100s_debe_reg_t * debe = (struct f1c100s_debe_reg_t *)lcd_pdat->virtdebe;
	write32((virtual_addr_t)&debe->layer0_attr1_ctrl, format << 8);
		  write32((virtual_addr_t)&debe->layer0_stride, ((lcd_timing.width) << stride));
	/*switch(Layer)
	{
		case 0:
			write32((virtual_addr_t)&debe->layer0_attr1_ctrl, format << 8);
		  write32((virtual_addr_t)&debe->layer0_stride, ((lcd_timing.width) << stride));
			break;
		case 1:
			write32((virtual_addr_t)&debe->layer1_attr1_ctrl, format << 8);
			write32((virtual_addr_t)&debe->layer1_stride, ((lcd_timing.width) << stride));		
			break;
		case 2:
			write32((virtual_addr_t)&debe->layer2_attr1_ctrl, format << 8);	
			write32((virtual_addr_t)&debe->layer2_stride, ((lcd_timing.width) << stride));	
			break;
		case 3:
			write32((virtual_addr_t)&debe->layer3_attr1_ctrl, format << 8);		
			write32((virtual_addr_t)&debe->layer3_stride, ((lcd_timing.width) << stride));	
			break;
	}	*/
}
static int video_layer_inx=0;

/*------------------------
设置视频窗口位置与大小
------------------------*/
void set_video_window(int x,int y,int w,int h)
{
	Set_LayerX_window(video_layer_inx,x,y,w,h);
}
/*------------------------
使能视频通道
------------------------*/
void Enable_Layer_Video(void)
{
	Enable_LayerX_Video(video_layer_inx);
}

/*------------------------
设置层优先级
Priority=0-3	[3>2>1>0]
------------------------*/
void debe_set_layer_priority(int layer,int Priority)
{
	struct f1c100s_debe_reg_t * debe = (struct f1c100s_debe_reg_t *)lcd_pdat->virtdebe;
	unsigned int val=read32((virtual_addr_t)&debe->layer1_attr0_ctrl);
	val&=~(0x3<<10);
	val|=(Priority<<10);
	write32((virtual_addr_t)&debe->layer1_attr0_ctrl,val);
}
/*TCON初始化*/
void Tcon_Init(void)
{
  struct f1c100s_tcon_reg_t *tcon=((struct f1c100s_tcon_reg_t *)lcd_pdat->virttcon);
	struct fb_f1c100s_pdata_t * pdat = lcd_pdat;
	int val;
	int bp, total;
/*设置VIDEO PLL时钟*/	
	C_BIT(CCU_Base_Address+0x010,31);
	write32(CCU_Base_Address+0x010,((lcd_timing.n-1)<<8)|((lcd_timing.m-1)<<0)|(3<<24));
	S_BIT(CCU_Base_Address+0x010,31);
	delay_ms(1);	
/*设置TCON时钟与复位*/
	S_BIT(CCU_BUS_CLK_GATING_REG1,4);
	S_BIT(CCU_TCON_CLK_REG,31);
	S_BIT(CCU_BUS_SOFT_RST_REG1,4);
	delay_ms(1);
	if(lcd_timing.mode!=7)//非TV
	{
	/*TCON配置*/
		C_BIT(TCON_Base_Address+0x40,0);
		val = (lcd_timing.v_front_porch + lcd_timing.v_back_porch + lcd_timing.v_sync_len);
		write32(TCON_Base_Address+0x40,((u64_t)0x1 << 31) |(val & 0x1f) << 4);	
		val = lcd_timing.f; // 5< DCLKDIV <96 时钟除数
		write32(TCON_Base_Address+0x44, ((u64_t)0xf << 28) | (val << 0));	
	/*设置宽高*/
		write32((virtual_addr_t)&tcon->tcon0_timing_active, ((lcd_timing.width - 1) << 16) | ((lcd_timing.height - 1) << 0));
	/*设置HT+HBP*/
		bp = lcd_timing.h_sync_len + lcd_timing.h_back_porch;
		total = (lcd_timing.width*lcd_timing.serial) + lcd_timing.h_front_porch + bp;
		write32((virtual_addr_t)&tcon->tcon0_timing_h, ((total - 1) << 16) | ((bp - 1) << 0));
	/*设置VT+VBP*/
		bp = lcd_timing.v_sync_len + lcd_timing.v_back_porch;
		total = lcd_timing.height + lcd_timing.v_front_porch + bp;
		write32((virtual_addr_t)&tcon->tcon0_timing_v, ((total * 2) << 16) | ((bp - 1) << 0));
	/*设置时钟宽度*/
		write32((virtual_addr_t)&tcon->tcon0_timing_sync, ((lcd_timing.h_sync_len - 1) << 16) | ((lcd_timing.v_sync_len - 1) << 0));
	/*设置RB交换-方便布线*/
	#if 0	
		S_BIT(TCON_Base_Address+0x40,23);
	#endif
	/*设置模式*/
		if(lcd_timing.mode>0)//rgb
		{
			write32((virtual_addr_t)&tcon->tcon0_hv_intf, 0);
			write32((virtual_addr_t)&tcon->tcon0_cpu_intf, 0);	
		}
		else//cpu
		{
			//设置为8080模式
			write32(TCON_Base_Address+0x40,read32(TCON_Base_Address+0x40)|(1)<<24);	
			//CPU模式
			write32(TCON_Base_Address+0x60,(u64_t)(lcd_timing.cpu_mode)<<29 | (u64_t)(1)<<28);
		}
		//设置输入源
		write32(TCON_Base_Address+0x40,read32(TCON_Base_Address+0x40)|(0)<<0);//[3-白色数据][2-DMA][0-DE]	
		//FRM
		if(pdat->frm_pixel == 18 || pdat->frm_pixel == 16)
		{
			write32((virtual_addr_t)&tcon->tcon0_frm_seed[0], 0x11111111);
			write32((virtual_addr_t)&tcon->tcon0_frm_seed[1], 0x11111111);
			write32((virtual_addr_t)&tcon->tcon0_frm_seed[2], 0x11111111);
			write32((virtual_addr_t)&tcon->tcon0_frm_seed[3], 0x11111111);
			write32((virtual_addr_t)&tcon->tcon0_frm_seed[4], 0x11111111);
			write32((virtual_addr_t)&tcon->tcon0_frm_seed[5], 0x11111111);
			write32((virtual_addr_t)&tcon->tcon0_frm_table[0], 0x01010000);
			write32((virtual_addr_t)&tcon->tcon0_frm_table[1], 0x15151111);
			write32((virtual_addr_t)&tcon->tcon0_frm_table[2], 0x57575555);
			write32((virtual_addr_t)&tcon->tcon0_frm_table[3], 0x7f7f7777);
			write32((virtual_addr_t)&tcon->tcon0_frm_ctrl, (pdat->frm_pixel == 18) ? (((u64_t)1 << 31) | (0 << 4)) : (((u64_t)1 << 31) | (5 << 4)));
		}
		//极性控制 
		val = (1 << 28);	
		if(!lcd_timing.h_sync_active)	val |= (1 << 25);	
		if(!lcd_timing.v_sync_active)	val |= (1 << 24);	
		if(!lcd_timing.den_active)		val |= (1 << 27);		
		if(!lcd_timing.clk_active)		val |= (1 << 26);	
		write32((virtual_addr_t)&tcon->tcon0_io_polarity, val);
		//触发控制关
		write32((virtual_addr_t)&tcon->tcon0_io_tristate, 0);
	}else 
	{ //TV
		write32(TCON_Base_Address+0x40,read32(TCON_Base_Address+0x40)|(3)<<0);//[3-白色数据][2-DMA][0-DE]	
	}
}
/*debe配置*/
void Debe_Init(void)
{
	struct f1c100s_debe_reg_t * debe = (struct f1c100s_debe_reg_t *)lcd_pdat->virtdebe;
	struct fb_f1c100s_pdata_t * pdat = lcd_pdat;
//时钟与复位
	S_BIT(CCU_BUS_CLK_GATING_REG1,12);
	S_BIT(CCU_BUS_SOFT_RST_REG1,12);
	delay_ms(1);
//使能DEBE
	S_BIT((virtual_addr_t)&debe->mode,0);
	write32((virtual_addr_t)&debe->disp_size, (((pdat->height) - 1) << 16) | (((pdat->width) - 1) << 0));
//设置层0参数
	write32((virtual_addr_t)&debe->layer0_size, (((pdat->height) - 1) << 16) | (((pdat->width) - 1) << 0));
	write32((virtual_addr_t)&debe->layer0_stride, ((pdat->width) << 4));//[RGB565=4][ARGB8888=5]
	write32((virtual_addr_t)&debe->layer0_addr_low32b, (u32)(pdat->vram[0]) << 3);
	write32((virtual_addr_t)&debe->layer0_addr_high4b, (u32)(pdat->vram[0]) >> 29);
	write32((virtual_addr_t)&debe->layer0_attr1_ctrl, 0x5 << 8);//[RGB565=0x5][ARGB8888=0xA]
	S_BIT((virtual_addr_t)&debe->mode,8);//层0使能
  if((pdat->vram[1]!=0)&&(pdat->vram[0]!=pdat->vram[1]))
	{		
//设置层1参数
		write32((virtual_addr_t)&debe->disp_size, (((pdat->height) - 1) << 16) | (((pdat->width) - 1) << 0));
		write32((virtual_addr_t)&debe->layer1_size, (((pdat->height) - 1) << 16) | (((pdat->width) - 1) << 0));
		write32((virtual_addr_t)&debe->layer1_stride, ((pdat->width) <<5));//[RGB565=4][ARGB8888=5]
		write32((virtual_addr_t)&debe->layer1_addr_low32b, (u32)(pdat->vram[1]) << 3);
		write32((virtual_addr_t)&debe->layer1_addr_high4b, (u32)(pdat->vram[1]) >> 29);
		write32((virtual_addr_t)&debe->layer1_attr1_ctrl, 0x0A << 8);//[RGB565=0x5][ARGB8888=0xA]
		
		debe_set_layer_priority(0,1);//设置优先级 层0>层1
		S_BIT((virtual_addr_t)&debe->layer1_attr0_ctrl,15);//1: select Pipe 1 需要透明叠加时需要输入不同管道  层0=pipe0 层1=pipe1
		
		S_BIT((virtual_addr_t)&debe->mode,9);//层1使能		
	}
//加载
	S_BIT((virtual_addr_t)&debe->reg_ctrl,0);
//DEBE 开始
	S_BIT((virtual_addr_t)&debe->mode,1);
}

/*tcon使能*/
void f1c100s_tcon_enable(void)
{
	struct f1c100s_tcon_reg_t * tcon = (struct f1c100s_tcon_reg_t *)lcd_pdat->virttcon;
	u32_t val;

	val = read32((virtual_addr_t)&tcon->ctrl);
	val |= ((u64_t)1 << 31);
	write32((virtual_addr_t)&tcon->ctrl, val); 	
}

/*设置地址*/
void f1c100s_lcd_set_addr(void)
{
	lcd_pdat=&__lcd_pdat;
	//设置地址
	lcd_pdat->virtdebe = F1C100S_DEBE_BASE;
	lcd_pdat->virttcon = F1C100S_TCON_BASE;
}	
/*LCD配置*/
void F1C100S_LCD_Config(int width,int height,unsigned int *buff1,unsigned int *buff2)
{
	//设置地址
  f1c100s_lcd_set_addr();
	//LCD宽高
	lcd_pdat->width = width;
	lcd_pdat->height = height;
	//FRM-抖动功能-当显存为RGB888,IO定义为RGB565或RGB666时输出接近RGB888的效果
	if(lcd_timing.mode==1)lcd_pdat->frm_pixel = 16;
	else if(lcd_timing.mode==2)lcd_pdat->frm_pixel = 18;	
	else lcd_pdat->frm_pixel = 0;/*其它为0*/	
	//设置缓存
	lcd_pdat->vram[0] = buff1;
	lcd_pdat->vram[1] = buff2;	
	//清寄存器
	if((read32(TCON_Base_Address+0x00)&((u64_t)1<<31))==0) //判断LCD开启
	{
		for(int i = 0x0800; i < 0x1000; i += 4)
		write32(F1C100S_DEBE_BASE + i, 0);
	}
	//debe+tcon配置	
	Debe_Init();		
	Tcon_Init();
	f1c100s_tcon_enable();
}
/*LCD IO初始化*/
void LCD_IO_Init(void)
{
	int i=0;
	//
	if(lcd_timing.mode>0)//RGB
	{
		if(lcd_timing.mode==1)//RGB565	
		{
			for(i=0;i<=21;i++)
			{
				if((i==0)||(i==12))continue;
				GPIO_Congif(GPIOD,GPIO_Pin_0+i,GPIO_Mode_010,GPIO_PuPd_NOPULL);
			}
		}
		else if(lcd_timing.mode==2)//RGB666
		{
			for(i=0;i<=21;i++)GPIO_Congif(GPIOD,GPIO_Pin_0+i,GPIO_Mode_010,GPIO_PuPd_NOPULL);
		}
		else if(lcd_timing.mode==3)//RGB888	
		{
			for(i=0;i<=21;i++)GPIO_Congif(GPIOD,GPIO_Pin_0+i,GPIO_Mode_010,GPIO_PuPd_NOPULL);
			for(i=0;i<=5;i++)GPIO_Congif(GPIOE,GPIO_Pin_0+i,GPIO_Mode_011,GPIO_PuPd_NOPULL);
		}
		else if(lcd_timing.mode==4)//RGB8位串行
		{
			for(i=18;i<=21;i++)GPIO_Congif(GPIOD,GPIO_Pin_0+i,GPIO_Mode_010,GPIO_PuPd_NOPULL);
			for(i=1;i<=8;i++)GPIO_Congif(GPIOD,GPIO_Pin_0+i,GPIO_Mode_010,GPIO_PuPd_NOPULL);	
		}
		else if(lcd_timing.mode==5)//CCIR656
		{
			GPIO_Congif(GPIOD,GPIO_Pin_18,GPIO_Mode_010,GPIO_PuPd_NOPULL);//只需要一个时钟线
			for(i=1;i<=8;i++)GPIO_Congif(GPIOD,GPIO_Pin_0+i,GPIO_Mode_010,GPIO_PuPd_NOPULL);	
		}
	}else//MCU
	{
		for(i=18;i<=21;i++)GPIO_Congif(GPIOD,GPIO_Pin_0+i,GPIO_Mode_010,GPIO_PuPd_NOPULL);			
		if((lcd_timing.cpu_mode==6)||(lcd_timing.cpu_mode==7))//MCU 8位
		{
			for(i=1;i<=8;i++)GPIO_Congif(GPIOD,GPIO_Pin_0+i,GPIO_Mode_010,GPIO_PuPd_NOPULL);	
		}
		else if((lcd_timing.cpu_mode==1)||(lcd_timing.cpu_mode==2)||(lcd_timing.cpu_mode==3)||(lcd_timing.cpu_mode==4))//MCU 16位
		{
			for(i=1;i<=8;i++)GPIO_Congif(GPIOD,GPIO_Pin_0+i,GPIO_Mode_010,GPIO_PuPd_NOPULL);
			for(i=10;i<=17;i++)GPIO_Congif(GPIOD,GPIO_Pin_0+i,GPIO_Mode_010,GPIO_PuPd_NOPULL);
		}		
	}
}	
/*等待帧中断*/
void lcd_wait_vertical_blanking_interrupt(void) 
{
	if(read32(TCON_Base_Address+0x00)&((u64_t)1<<31)) //判断LCD开启
	{
		//sysprintf("The LCD has been initialized.\r\n");	
		C_BIT(TCON_Base_Address+0x04,15);
		while(1)
		{
			if(read32(TCON_Base_Address+0x04)&(1<<15))break;
		}
	}
}
/*LCD初始化*/
void Sys_LCD_Init(int width,int height,unsigned int *buff1,unsigned int *buff2)
{
#if 0 //多次初始化为了时钟连续性不用关时钟
	Tcon_Clk_Close();
	Debe_Clk_Close();
#endif
  lcd_wait_vertical_blanking_interrupt();//等待帧中断
	LCD_IO_Init();//IO初始化-如果第一次初始化IO与第二次初始化IO不一样，需要先关闭时钟再初始化
	F1C100S_LCD_Config(width,height,buff1,buff2);//TCON DEBE 初始化	
}
/*TCON中断*/
void TCON_ISR(void)
{
	//这里TCON_INT_REG0需要写0清除中断标志-参考数据手册183页
	C_BIT(TCON_Base_Address+0x04,15);//清除TCON0消隐中断
	
}
