#include "sys_clock.h"
#include "sys_tvd.h"
#include "sys_uart.h"
#include "sys_delay.h"
#include "sys_io.h"
#include "sys_interrupt.h"
#include "sys_cache.h"
//--------------
#include "sys_lcd_conf.h"
//--------------
#include "sys_lcd.h"
#include "sys_defe.h"
#include "sys_timer.h"

//输出宽高
int TVD_OUT_W=720;
int TVD_OUT_H=480;

int TVD_IN_SEL=0;
int SAVE_BMP;//保存图像标志 1=开始保存;
#define TVD_REGS_BASE   0x01c0b000

void BSP_TVD_input_select(__u32 input);
void BSP_TVD_set_fmt(__u32 id, TVD_FMT_T fmt);

#define REG_RD32(reg)    (*((volatile __u32 *)(reg)))
#define REG_WR32(reg, value) (*((volatile __u32 *)(reg))  = (value))
	

__align(32) unsigned char buff_y[3][720*576];
__align(32) unsigned char buff_c[3][720*576];

/*
TVD帧中断
*/
static int buff_n=0;//缓存计数，用于切换缓存，
static int sn=0;	
void tvd_clear_v(void)//清值
{
	sn=0;
	buff_n=0;
}
/*
void IRQ_TVD_ISR(void)
{
static int CSItime=0,Stime=0;
	
int inx=0;		
			if(BSP_TVD_irq_status_get(0, TVD_FRAME_DONE))
	{
		BSP_TVD_irq_status_clear(0, TVD_FRAME_DONE);
	
			sysprintf("TVDin Fram %d ms\r\n",(Read_time_ms()-CSItime));	
			CSItime = Read_time_ms();
	}
}*/
void BSP_TVD_capture_on2(void)
{
	BSP_TVD_capture_on(TVD_IN_SEL);//停止采集
}

void BSP_TVD_init(void)
{
    //reg for set once here
    REG_WR32(TVD_REGS_BASE+0x0088,0x04000000);
}

void BSP_TVD_config(__u32 interface, __u32 system, __u32 format)
{
    sysprintf("system:%d\r\n", system);
    
    //global reg set here
    REG_WR32(TVD_REGS_BASE+0x0e04,0x8002AAA8);//analog register,set
    REG_WR32(TVD_REGS_BASE+0x0e2c,0x00110000);
    REG_WR32(TVD_REGS_BASE+0x0040,0x04000310);//?
                    
    //reset tvd
    REG_WR32(TVD_REGS_BASE+0x0000,0x00000000);
    delay_ms(1000);//need delay here for tvd reset
    REG_WR32(TVD_REGS_BASE+0x0000, 0x00001f00);

    //IP config
    REG_WR32(TVD_REGS_BASE+0x0014, 0x20000000); //2000_0000    2400_0000    
    REG_WR32(TVD_REGS_BASE+0x0f24, 0x0682810a); //DISABLE AGC GATE KILL!!!!!!!!!!!!!!!!!
    REG_WR32(TVD_REGS_BASE+0x0f28, 0x00006440); //0000_6440    0000_5838    
    REG_WR32(TVD_REGS_BASE+0x0f4c, 0x0e70106c); //NO CLAMP DURING VSYNC!!!!!!!!!!!!!!!!!
    REG_WR32(TVD_REGS_BASE+0x0f54, 0x00000000); //
    REG_WR32(TVD_REGS_BASE+0x0f58, 0x00000082); //      
    REG_WR32(TVD_REGS_BASE+0x0f6c, 0x00fffad0); //YC separation config
    REG_WR32(TVD_REGS_BASE+0x0f70, 0x0000a010);//0x00002010); 

    if(interface==TVD_SOURCE_CVBS){//composite
        switch(system)
        {
            case TVD_SOURCE_NTSC:
                REG_WR32(TVD_REGS_BASE+0x0008, 0x00010001);
                REG_WR32(TVD_REGS_BASE+0x000c, 0x00202068); //adjust luma brightness
                REG_WR32(TVD_REGS_BASE+0x0010, 0x00300080); //just statruation and peak gain
                REG_WR32(TVD_REGS_BASE+0x0018, 0x21f07c1f); //21f0_7c1f    262E_8BA2 Chroma DTO                
                REG_WR32(TVD_REGS_BASE+0x001c, 0x00820022); //hactive and vactive start
                REG_WR32(TVD_REGS_BASE+0x0f08, 0x00590100); //notch width 0059_0100
                REG_WR32(TVD_REGS_BASE+0x0f0c, 0x00000010); //YC sep
                REG_WR32(TVD_REGS_BASE+0x0f10, 0x008A32DD); //sync height
                REG_WR32(TVD_REGS_BASE+0x0f14, 0x800000a0); //
                REG_WR32(TVD_REGS_BASE+0x0f1c, 0x008A0000); //chroma AGC target
                REG_WR32(TVD_REGS_BASE+0x0f2c, 0x0000CB74);
                REG_WR32(TVD_REGS_BASE+0x0f44, 0x00004632); //burst gate
                REG_WR32(TVD_REGS_BASE+0x0f74, 0x000003c3); //chroma edge enhance
                REG_WR32(TVD_REGS_BASE+0x0f80, 0x00500000); //hactive width
                REG_WR32(TVD_REGS_BASE+0x0f84, 0x00610000); //vactive height
                REG_WR32(TVD_REGS_BASE+0x0000, 0x00000001);//test = 0x80001f3f

                if( TVD_MB_YUV420==format )
                {
                    BSP_TVD_set_width(0,704);
                    BSP_TVD_set_width_jump(0,704);
                    BSP_TVD_set_height(0,224);
                    BSP_TVD_set_ver_start(0,0x22+(480-448)/2);
                }
                else
                {
                    BSP_TVD_set_width(0,720);
                    BSP_TVD_set_width_jump(0,720);
                    BSP_TVD_set_height(0,240);              
                }   

                break;
            case TVD_SOURCE_PAL:
                REG_WR32(TVD_REGS_BASE+0x0008, 0x01101001);
                REG_WR32(TVD_REGS_BASE+0x000c, 0x00202068); //adjust luma brightness
                REG_WR32(TVD_REGS_BASE+0x0010, 0x00300050); 
                REG_WR32(TVD_REGS_BASE+0x0018, 0x2a098acb); //chroma dto
                REG_WR32(TVD_REGS_BASE+0x001c, 0x0087002a); //hactive and vactive start     
                REG_WR32(TVD_REGS_BASE+0x0f08, 0x11590902); //disable black level correction for 7.5 blank-to-black setup cagc en
                REG_WR32(TVD_REGS_BASE+0x0f0c, 0x00000016); //YC sep
                REG_WR32(TVD_REGS_BASE+0x0f10, 0x008a32ec); //
                REG_WR32(TVD_REGS_BASE+0x0f14, 0x800000a0); //adjust YC delay               
                REG_WR32(TVD_REGS_BASE+0x0f1c, 0x00930000); //chroma AGC target
                REG_WR32(TVD_REGS_BASE+0x0f2c, 0x00000d74);             
                REG_WR32(TVD_REGS_BASE+0x0f44, 0x0000412d); //burst gate
                REG_WR32(TVD_REGS_BASE+0x0f74, 0x00000343); //      
                REG_WR32(TVD_REGS_BASE+0x0f80, 0x00500000); //hactive width
                REG_WR32(TVD_REGS_BASE+0x0f84, 0x00c10000); //vactive height    
                REG_WR32(TVD_REGS_BASE+0x0000, 0x00000001); //

                if( TVD_MB_YUV420==format )
                {
                    BSP_TVD_set_width(0,704);
                    BSP_TVD_set_width_jump(0,704);
                    BSP_TVD_set_height(0,288);              
                }
                else
                {
                    BSP_TVD_set_width(0,720);
                    BSP_TVD_set_width_jump(0,720);
                    BSP_TVD_set_height(0,288);              
                }							
                break;
            case TVD_SOURCE_PAL_M:
                REG_WR32(TVD_REGS_BASE+0x0008, 0x00002001);
                REG_WR32(TVD_REGS_BASE+0x000c, 0x00002080); 
                REG_WR32(TVD_REGS_BASE+0x0010, 0x00300080); //just statruation and peak gain                
                REG_WR32(TVD_REGS_BASE+0x0018, 0x21E6EFE3); //21f0_7c1f    262E_8BA2 Chroma DTO
                REG_WR32(TVD_REGS_BASE+0x001c, 0x00820022); //hactive and vactive start                                         
                REG_WR32(TVD_REGS_BASE+0x0f08, 0x00590100); //notch width 0059_0100
                REG_WR32(TVD_REGS_BASE+0x0f0c, 0x00000040);
                REG_WR32(TVD_REGS_BASE+0x0f10, 0x008A32DD); //sync height
                REG_WR32(TVD_REGS_BASE+0x0f14, 0x800000a0); //
                REG_WR32(TVD_REGS_BASE+0x0f1c, 0x008a0000); //chroma AGC target
                REG_WR32(TVD_REGS_BASE+0x0f2c, 0x0000CB74);             
                REG_WR32(TVD_REGS_BASE+0x0f44, 0x00004632); //burst gate    
                REG_WR32(TVD_REGS_BASE+0x0f74, 0x000003c3); //chroma edge enhance                   
                REG_WR32(TVD_REGS_BASE+0x0f80, 0x00500000); //hactive width
                REG_WR32(TVD_REGS_BASE+0x0f84, 0x00610000); //vactive height
                REG_WR32(TVD_REGS_BASE+0x0000, 0x00000001);//test = 0x80001f3f
                if( TVD_MB_YUV420==format )
                {
                    BSP_TVD_set_width(0,704);
                    BSP_TVD_set_width_jump(0,704);
                    BSP_TVD_set_height(0,224);  
                    REG_WR32(TVD_REGS_BASE+0x001c, 0x00820032); // vertical start 0x22->(0x22+(480-448)/2)
                }
                else
                {
                    BSP_TVD_set_width(0,720);
                    BSP_TVD_set_width_jump(0,720);
                    BSP_TVD_set_height(0,240);              
                }   
                break;
            case TVD_SOURCE_PAL_N:
                REG_WR32(TVD_REGS_BASE+0x0008, 0x01103001);
                REG_WR32(TVD_REGS_BASE+0x000c, 0x00002080); //adjust luma brightness    
                REG_WR32(TVD_REGS_BASE+0x0010, 0x00300080); //just statruation and peak gain    
                REG_WR32(TVD_REGS_BASE+0x0018, 0x21F69446); //chroma dto                
                REG_WR32(TVD_REGS_BASE+0x001c, 0x00870026); //hactive and vactive start                                     
                REG_WR32(TVD_REGS_BASE+0x0f08, 0x11590902); //disable black level correction for 7.5 blank-to-black setup cagc en
                REG_WR32(TVD_REGS_BASE+0x0f0c, 0x00000040);
                REG_WR32(TVD_REGS_BASE+0x0f10, 0x008a32ec); //
                REG_WR32(TVD_REGS_BASE+0x0f14, 0x800000a0); //adjust YC delay
                REG_WR32(TVD_REGS_BASE+0x0f1c, 0x00dc0000); //chroma AGC target
                REG_WR32(TVD_REGS_BASE+0x0f2c, 0x00000d74);             
                REG_WR32(TVD_REGS_BASE+0x0f44, 0x00004632); //burst gate
                REG_WR32(TVD_REGS_BASE+0x0f74, 0x00000343); //chroma peak
                REG_WR32(TVD_REGS_BASE+0x0f80, 0x00500000); //hactive width
                REG_WR32(TVD_REGS_BASE+0x0f84, 0x00c10000); //vactive height
                REG_WR32(TVD_REGS_BASE+0x0000, 0x00000001); //
                if( TVD_MB_YUV420==format )
                {
                    BSP_TVD_set_width(0,704);
                    BSP_TVD_set_width_jump(0,704);
                    BSP_TVD_set_height(0,288);              
                }
                else
                {
                    BSP_TVD_set_width(0,720);
                    BSP_TVD_set_width_jump(0,720);
                    BSP_TVD_set_height(0,288);              
                }   
                break;
            case TVD_SOURCE_SECAM:
                REG_WR32(TVD_REGS_BASE+0x0008, 0x01104001);
                REG_WR32(TVD_REGS_BASE+0x000c, 0x00002080); //adjust luma brightness                
                REG_WR32(TVD_REGS_BASE+0x0010, 0x003100b0);
                REG_WR32(TVD_REGS_BASE+0x0018, 0x28A33BB2); //chroma dto    
                REG_WR32(TVD_REGS_BASE+0x001c, 0x00870026); //hactive and vactive start                         
                REG_WR32(TVD_REGS_BASE+0x0f08, 0x11590902); //disable black level correction for 7.5 blank-to-black setup cagc en
                REG_WR32(TVD_REGS_BASE+0x0f0c, 0x00000040);
                REG_WR32(TVD_REGS_BASE+0x0f10, 0x008a32ec); //
                REG_WR32(TVD_REGS_BASE+0x0f14, 0x800000a0); //adjust YC delay
                REG_WR32(TVD_REGS_BASE+0x0f1c, 0x00dc0000); //chroma AGC target
                REG_WR32(TVD_REGS_BASE+0x0f2c, 0x00000d74);             
                REG_WR32(TVD_REGS_BASE+0x0f44, 0x00005036); //burst gate
                REG_WR32(TVD_REGS_BASE+0x0f74, 0x00000343); //chroma peak
                REG_WR32(TVD_REGS_BASE+0x0f80, 0x00500000); //hactive width
                REG_WR32(TVD_REGS_BASE+0x0f84, 0x00c10000); //vactive height
                REG_WR32(TVD_REGS_BASE+0x0000, 0x00000001); //
                if( TVD_MB_YUV420==format )
                {
                    BSP_TVD_set_width(0,704);
                    BSP_TVD_set_width_jump(0,704);
                    BSP_TVD_set_height(0,288);              
                }
                else
                {
                    BSP_TVD_set_width(0,720);
                    BSP_TVD_set_width_jump(0,720);
                    BSP_TVD_set_height(0,288);              
                }   

                break;

        }
				
//////switch
//REG_WR32(TVD_REGS_BASE+0x0504,0x00000000);
//REG_WR32(TVD_REGS_BASE+0x052c,0x00110000);
////1 channel cvbs
//REG_WR32(TVD_REGS_BASE+0x0500,0x00000111);
//REG_WR32(TVD_REGS_BASE+0x0000,0x00000321);
//// default open all 4 channels if you don't care power consumption
//REG_WR32(TVD_REGS_BASE+0x0500,0x00000f11);

//REG_WR32(TVD_REGS_BASE+0x0e2c,0x60000000);
    }

    
    #if 0
    {
        __u32 v;
        v = REG_RD32(TVD_REGS_BASE+0x0f20);
        sysprintf("1-----0x%x\r\n", REG_RD32(TVD_REGS_BASE+0x0f20));
        v &= ~((1<<6)|(1<<7));
        v |= ((0<<6)|(1<<7));
        REG_WR32(TVD_REGS_BASE+0x0f20,v);
        sysprintf("1-----0x%x\r\n", REG_RD32(TVD_REGS_BASE+0x0f20));
    }
    #endif
}

void BSP_TVD_set_width(__u32 id,__u32 w)
{
    __u32 reg_val;
    reg_val = REG_RD32(TVD_REGS_BASE+0x008c);
    reg_val &= ~(0xfff<<0);
    reg_val |= ((w>720)?720:w)<<0;
    REG_WR32(TVD_REGS_BASE+0x008c, reg_val);
}

void BSP_TVD_set_width_jump(__u32 id,__u32 j)
{
    REG_WR32(TVD_REGS_BASE+0x0090, j);
}

void BSP_TVD_set_height(__u32 id,__u32 h)
{
    __u32 reg_val;
    reg_val = REG_RD32(TVD_REGS_BASE+0x008c);
    reg_val &= ~(0x7ff<<16);
    reg_val |= h<<16;
    REG_WR32(TVD_REGS_BASE+0x008c, reg_val);
}

void BSP_TVD_set_ver_start(__u32 id,__u32 v_start)
{
    __u32 reg_val;
    reg_val = REG_RD32(TVD_REGS_BASE+0x001c);
    reg_val &= ~(0x7ff<<0);
    reg_val |= (v_start&0x7ff)<<0;  
    REG_WR32(TVD_REGS_BASE+0x001c, reg_val);
}

void BSP_TVD_irq_enable(__u32 id,tvd_irq_t irq)
{
    __u32 reg_val;
    switch(irq){
        case TVD_FRAME_DONE:
            reg_val = REG_RD32(TVD_REGS_BASE+0x009c);
            reg_val |= 1<<(24);
            REG_WR32(TVD_REGS_BASE+0x009c, reg_val);
            break;
        case TVD_LOCK:
            break;
        case TVD_UNLOCK:
            break;
    }
}


__u32 BSP_TVD_irq_status_get(__u32 id,tvd_irq_t irq)
{
    __u32 reg_val, ret = 0;
    switch(irq){
        case TVD_FRAME_DONE:
            reg_val = REG_RD32(TVD_REGS_BASE+0x0094);
            ret = (reg_val>>24)&1;
            break;
    }
    return ret;
}

void BSP_TVD_irq_status_clear(__u32 id,tvd_irq_t irq)
{
    __u32 reg_val;
    switch(irq){
        case TVD_FRAME_DONE:
            reg_val = 1<<24;
            REG_WR32(TVD_REGS_BASE+0x0094, reg_val);
            break;
    }
}

void BSP_TVD_capture_on(__u32 id)
{
    __u32 reg_val;
    reg_val = REG_RD32(TVD_REGS_BASE+0x0088);
    reg_val |= 1<<0;
    REG_WR32(TVD_REGS_BASE+0x0088, reg_val);
}

void BSP_TVD_capture_off(__u32 id)
{
    __u32 reg_val;
    reg_val = REG_RD32(TVD_REGS_BASE+0x0088);
    reg_val &= ~(1<<0);
    REG_WR32(TVD_REGS_BASE+0x0088, reg_val);
}

void BSP_TVD_set_addr_y(__u32 id,__u32 addr)
{
    __u32 reg_val;
    REG_WR32(TVD_REGS_BASE+0x0080, addr);
    reg_val = REG_RD32(TVD_REGS_BASE+0x0088);
    reg_val |= (1<<28);
    REG_WR32(TVD_REGS_BASE+0x0088, reg_val);
}

void BSP_TVD_set_addr_c(__u32 id,__u32 addr)
{
    __u32 reg_val;
    REG_WR32(TVD_REGS_BASE+0x0084, addr);
    reg_val = REG_RD32(TVD_REGS_BASE+0x0088);
    reg_val |= (1<<28);
    REG_WR32(TVD_REGS_BASE+0x0088, reg_val);
}

//void BSP_TVD_set_fmt(__u32 id, tvd_fmt_t fmt)
void BSP_TVD_set_fmt(__u32 id, TVD_FMT_T fmt)
{
    __u32 reg_val;
    reg_val = REG_RD32(TVD_REGS_BASE+0x0088);
    switch(fmt){
        case TVD_PL_YUV422:
            reg_val &= ~(1<<24);
            reg_val |= 1<<4;
            break;
        case TVD_PL_YUV420:
            reg_val &= ~(1<<24);
            reg_val &= ~(1<<4);
            break;
        case TVD_MB_YUV420:
            reg_val |= 1<<24;
            reg_val &= ~(1<<4);
            break;
    }
    REG_WR32(TVD_REGS_BASE+0x0088, reg_val);
}

__u32 BSP_TVD_get_status(__u32 id)
{
    __u32 reg_val = 0;
    __u32 det = 0;
    __u32 system = 0;
    reg_val = REG_RD32(TVD_REGS_BASE+0x0e40);
    if(reg_val&1)
    {
        det = 0;// no signal detect
    }
    else
    {
        det = 1;// detect tv signal
    }
    if(reg_val&(1<<18))
    {
        system = 1;//get system = pal
    }
    else{
        system = 0;//get system = ntsc
    }
    return ((det<<0)+(system<<4));//bit0=det bit4=system
}

void BSP_TVD_input_select(__u32 input)
{
    __u32 reg_val;
    if(input==0)
    {
        reg_val = REG_RD32(TVD_REGS_BASE+0x0e04);
        reg_val &= ~(1<<0);
        REG_WR32(TVD_REGS_BASE+0x0e04, reg_val);    
    }
    else
    {
        reg_val = REG_RD32(TVD_REGS_BASE+0x0e04);
        reg_val |= (1<<0);
        REG_WR32(TVD_REGS_BASE+0x0e04, reg_val);        
    }
}

/*
f1c100s TVD CLK
*/
void f1c100s_clk_init(void)
{
#define TVD_CLK_REG (0x01C20000+0x124)
#define DRAM_GATING_REG (0x01C20000+0x100)	
	Open_Dev_Clock(TVD_CLOCK);
	//
	S_BIT(DRAM_GATING_REG,3);//DRAM TVD_DCLK_GATING
	//配置时钟到270MHZ
	C_BIT(0x01C20000+0x010,31);
	//write32(0x01C20000+0x010,((0x62)<<8)|((0x7)<<0)|(1<<24)|(1<<25));
	write32(0x01C20000+0x010,(0x62)<<8);
	S_BIT(0x01C20000+0x010,31);
	delay_ms(1);
	//配置时钟到[270MHZ/10]=27MHZ*********************************************************************************************************
	write32(TVD_CLK_REG,(0<<24));//000: PLL_VIDEO(1X) 001: OSC24M 010: PLL_VIDEO(2X)
	write32(TVD_CLK_REG,read32(TVD_CLK_REG)|(9<<0));//0000: CLK_DIV_RATIO_M
	S_BIT(TVD_CLK_REG,31);//SCLK_GATING
	Open_Dev_Clock(TVD_CLOCK);
}

/*
f1c100s TVD初始化
channel 通道
mode 模式
*/

void f1c100s_tvd_init(int Channel,int Mode)
{
	TVD_IN_SEL=Channel;
	f1c100s_clk_init();
	BSP_TVD_init();
	BSP_TVD_config(TVD_SOURCE_CVBS,Mode,TVD_PL_YUV422);
	BSP_TVD_input_select(TVD_IN_SEL);		
	BSP_TVD_set_fmt(TVD_IN_SEL, TVD_PL_YUV422);		
	BSP_TVD_set_addr_y(TVD_IN_SEL,(u32)buff_y[0]);
	BSP_TVD_set_addr_c(TVD_IN_SEL,(u32)buff_c[0]);
//开启中断	
//	
	
	BSP_TVD_capture_on(TVD_IN_SEL);
}
/*
TVD下DEFE配置
*/
void TvdDefeConfig(int Mode)
{
	Defe_Init();
	Defe_Config(Mode);//YUV_to_ARGB
	Defe_Start();	
}
/*
f1c100s_tvd 配置
channel 通道
mode 模式
*/
void TVD_Config(int Channel,int Mode)
{
	//sysprintf("TVD Config...\r\n");	
	/*if(Mode==TVD_SOURCE_PAL)
	{
		TVD_OUT_W=720;
		TVD_OUT_H=576;	
	}else
	{
		TVD_OUT_W=720;
		TVD_OUT_H=480;
	}	*/
	f1c100s_tvd_init(Channel,Mode);	
  TvdDefeConfig(Mode);
}
/*
自动模式切换
*/
int mode=TVD_SOURCE_NTSC,tmode=TVD_SOURCE_NTSC;
void f1c100s_tvd_AutoMode(void)
{
int res;
		res=BSP_TVD_get_status(0);
		if(res&0xf)
		{
			if((res>>4)&0xf)
			{
				mode=TVD_SOURCE_PAL;
				sysprintf("TV in [PAL]...\r\n");
			}
			else 
			{
				mode=TVD_SOURCE_NTSC;
				sysprintf("TV in [NTSC]...\r\n");
			}
		}else sysprintf("NO TV in...\r\n");
    if(mode!=tmode)//切换模式
		{
			sysprintf("Mode Conversion...\r\n");
			TVD_Config(TVD_IN_SEL,mode);
			tmode=mode;
		}
}
/*f1c100s_tvd_Init
channel 通道只能选择0与1
mode 模式
*/
void f1c100s_tvd_Init(int Channel,int Mode)
{
	sysprintf("TVD Init...\r\n");
	mode=Mode;
	tmode=Mode;
  TVD_Config(Channel,Mode);
}

