#include "sys_tve.h" 
#include "sys_types.h"
#include "sys_uart.h"
#include "sys_io.h"
#include "sys_delay.h"
#include "sys_lcd.h"
#include "stdlib.h"
//--------------
#include "sys_lcd_conf.h"
//--------------

__u32 tve_reg_base = 0;
__u8 TVE_dac_set_de_bounce(__u8 index,__u32 times);
__u8 TVE_dac_int_disable(__u8 index);
__s32 TVE_close(void);


__s32 TVE_set_reg_base( __u32 address)
{
	tve_reg_base = address;
	return 0;
}

__u32 TVE_get_reg_base(void)
{
	return tve_reg_base;
}

//tve
// init module
////////////////////////////////////////////////////////////////////////////////
__s32  TVE_init(void)
{
	TVE_close();

	TVE_dac_set_de_bounce(0,0);
	TVE_dac_int_disable(0);

	return 0;
}

__s32 TVE_exit(void)
{
	TVE_dac_int_disable(0);

	return 0;
}

// open module
////////////////////////////////////////////////////////////////////////////////
__s32 TVE_open(__u32 sel)
{
    TVE_SET_BIT(TVE_000, 0x1<<0);

	return 0;
}

__s32 TVE_close(void)
{
    TVE_CLR_BIT(TVE_000, 0x1<<0);

    return 0;
}

__s32 TVE_set_tv_mode(__u32 sel, __u8 mode)
{
    __u32 value_08 = TVE_RUINT32(TVE_008);
    value_08 &= ~(0x3f<<16);//clear bit 17~25
    value_08 |= (3<<20);//
    value_08 |= (3<<18);//
	  value_08 |= (3<<16);//
    TVE_WUINT32(TVE_008, value_08);
    
	switch(mode)
	{
	case DISP_TV_MOD_PAL:
		TVE_WUINT32(TVE_004, 0x07030001);
		TVE_SET_BIT(TVE_004, sel<<30);//set 0x004 reg first, because write it will change other regs
		TVE_WUINT32(TVE_014, 0x008a0018);
		TVE_WUINT32(TVE_01C, 0x00160271);
		TVE_WUINT32(TVE_114, 0x0016447e);
		TVE_WUINT32(TVE_124, 0x000005a0);
		TVE_WUINT32(TVE_130, 0x800D000C);
		TVE_WUINT32(TVE_13C, 0x00000000);
		TVE_WUINT32(TVE_00C, 0x00000124);
		TVE_WUINT32(TVE_020, 0x00fc00fc);
		TVE_WUINT32(TVE_10C, 0x00004040);
		TVE_WUINT32(TVE_128, 0x00000000);//
		TVE_WUINT32(TVE_118, 0x0000e0e0);
		TVE_WUINT32(TVE_12C, 0x00000101);//
		break;


	case DISP_TV_MOD_NTSC:
		TVE_WUINT32(TVE_004, 0x07030000);
		TVE_SET_BIT(TVE_004, sel<<30);//set 0x004 reg first, because write it will change other regs
		TVE_WUINT32(TVE_014, 0x00760020);
		TVE_WUINT32(TVE_01C, 0x0016020d);
		TVE_WUINT32(TVE_114, 0x0016447e);
		TVE_WUINT32(TVE_124, 0x000005a0);
		TVE_WUINT32(TVE_130, 0x000e000c);
		TVE_WUINT32(TVE_13C, 0x00000000);
		TVE_WUINT32(TVE_00C, 0x00000124);
		TVE_WUINT32(TVE_020, 0x00f0011a);
		TVE_WUINT32(TVE_10C, 0x0000004f);
		TVE_WUINT32(TVE_110, 0x00000000);
		TVE_WUINT32(TVE_118, 0x0000a0a0);
		TVE_WUINT32(TVE_11C, 0x001000f0);
		TVE_WUINT32(TVE_128, 0x00000000);//
		TVE_WUINT32(TVE_12C, 0x00000101);//
	
		break;


	default:
		return 0;
	}

	return 0;
}
__s32 TVE_set_vga_mode(__u32 sel)
{
    TVE_WUINT32(TVE_004, 0x20000000);
	TVE_SET_BIT(TVE_004, sel<<30);
    TVE_WUINT32(TVE_008, 0x40031ac7);
    TVE_WUINT32(TVE_024, 0x00000000);
    
	return 0;
}
      
__u8 TVE_query_interface(__u8 index)
{
    __u8 sts = 0;
    __u32 readval;

    readval = TVE_RUINT32(TVE_038);
    sts = readval & (3<<(index*8));
    sts >>= (index*8);
    
    return sts;
}

__u8 TVE_query_int(void)
{
    __u8    sts = 0;
    __u32   readval;

    readval = TVE_RUINT32(TVE_034);
    sts = readval & 0x0f;

    return sts;
}
__s32 tve_low_enhance(__u32 sel, __u32 mode)
{
    if (0 == mode) {
        TVE_CLR_BIT(TVE_000, 0xf<<10); /* deflick off */
        TVE_SET_BIT( TVE_000, 0x5<<10); /* deflick is 5 */
        TVE_SET_BIT( TVE_00C, (u64_t)0x1<<31); /* lti on */
        TVE_SET_BIT( TVE_00C, 0x1<<16); /* notch off */
    } else if (1 == mode) {
        TVE_CLR_BIT(TVE_000, 0xf<<10);
        TVE_SET_BIT( TVE_000, 0x5<<10);
        TVE_SET_BIT( TVE_00C, (u64_t)0x1<<31);
        TVE_CLR_BIT(TVE_00C, 0x1<<16);
    } else if (2 == mode) {
        TVE_CLR_BIT(TVE_000, 0xf<<10);
        TVE_CLR_BIT(TVE_00C, (u64_t)0x1<<31);
        TVE_SET_BIT( TVE_00C, 0x1<<16);
    }
    return 0;
}

__u8  TVE_clear_int(void)
{
    __u32    sts = 0;
    __u32    readval;
    
    readval = TVE_RUINT32(TVE_034);
    sts = readval & 0x0f;
    TVE_WUINT32(TVE_034,sts);
    
    return 0;
}

//0:unconnected; 1:connected; 3:short to ground
__s32 TVE_get_dac_status(__u32 index)
{
    __u32   readval;

    readval = TVE_RUINT32(TVE_038);
    
    if(index == 0)
    {
        readval = (readval & 0x00000003);
    }
    else
    {
       sysprintf("ERR: There is only one dac!\n");
    }

    return readval;
}

__u8 TVE_dac_int_enable(__u8 index)
{
    __u32   readval;

    readval = TVE_RUINT32(TVE_030);
    readval |= (1<<(16+index));
    TVE_WUINT32(TVE_030,readval);
    
    return 0;
}

__u8 TVE_dac_int_disable(__u8 index)
{
    __u32   readval;

    readval = TVE_RUINT32(TVE_030);
    readval &= (~(1<<(16+index)));
    TVE_WUINT32(TVE_030,readval);
    
    return 0;
}

__u8 TVE_dac_autocheck_enable(__u8 index)
{
    __u32   readval;

    readval = TVE_RUINT32(TVE_030);
    readval |= (1<<index);
    TVE_WUINT32(TVE_030,readval);
    
    return 0;
}

__u8 TVE_dac_autocheck_disable(__u8 index)
{
    __u32   readval;

    readval = TVE_RUINT32(TVE_030);
    readval &= (~(1<<index));
    TVE_WUINT32(TVE_030,readval);
    
    return 0;
}

__u8 TVE_dac_enable(__u8 index)
{
    __u32   readval;

	if(index==0)
	{
    	readval = TVE_RUINT32(TVE_008);
    	TVE_SET_BIT(TVE_008, readval | (1<<index));
	}
	else
	{
		sysprintf("ERR: There is only one DAC to enable!\n");
	}
   
    return 0;
}

__u8 TVE_dac_disable(__u8 index)
{
    __u32   readval;

	if(index==0)
	{
    	readval = TVE_RUINT32(TVE_008);
		TVE_WUINT32(TVE_008,readval & (~(1<<index)));
	}
	else
	{
		sysprintf("ERR: There is only one DAC to disable!\n");
	}
    
    return 0;
} 

__s32 TVE_dac_set_source(__u32 index,__u32 source)
{
	
	sysprintf("ERR: DAC source can't be set\n");
    
    return 0;
}

__u8 TVE_dac_set_de_bounce(__u8 index,__u32 times)
{
    __u32   readval;

    readval = TVE_RUINT32(TVE_03C);

    if(index == 0)
    {
        readval = (readval & 0xfffffff0)|(times & 0xf);
    }
    else
    {
    	sysprintf("ERR: There is only one DAC to set de-bounce!\n");
        return 0;
    }
    TVE_WUINT32(TVE_03C,readval);
    
    return 0;
}

__u8 TVE_dac_get_de_bounce(__u8 index)
{
    __u8    sts = 0;
    __u32   readval;

    readval = TVE_RUINT32(TVE_03C);

    if(index == 0)
    {
        sts = readval & 0xf;
    }
    else
    {
    	sysprintf("ERR: There is only one DAC to get de-bounce!\r\n");
        return 0;
    }
    
    return sts;
}


/*——————————————————————————————————————————————————————————————————————————————
TCON1时钟初始化
*/
void TVE_Clock_Open(void)
{
  #define TVE_CLK_REG (u32)(0x01C20000+0x120)
	//设置video时钟到270MHZ
	C_BIT(CCU_Base_Address+0x010,31);
	write32(CCU_Base_Address+0x010,(0x62<<8));
	S_BIT(CCU_Base_Address+0x010,31);
	delay_ms(1);
	//使能TVE时钟
	write32(CCU_BUS_CLK_GATING_REG1,read32(CCU_BUS_CLK_GATING_REG1)|(1)<<10);
	//设置时钟
	write32(TVE_CLK_REG,read32(TVE_CLK_REG)|((u64_t)(1)<<31) | (1)<<15 | (9)<<0 | (1)<<8 | (0)<<24);//270MHZ 十分频 27MHZ时钟 
	//使能TVE复位
	write32(CCU_BUS_SOFT_RST_REG1,read32(CCU_BUS_SOFT_RST_REG1)|((1)<<10));
	delay_ms(1);
}
/*——————————————————————————————————————————————————————————————————————————————
TCON1初始化
*/
int TCON1_Init(__u8 mode)
{	
	int b_interlace,src_x,src_y,scl_x,scl_y,out_x,out_y;
	int ht,hbp,vt,vbp,vspw,hspw; 	
	int vblank_len=0;
	int start_delay=0;	
	if(mode==DISP_TV_MOD_PAL)
	{	
		b_interlace   = 1;
		src_x       = 720;
		src_y       = 288;
		scl_x       = 720;
		scl_y       = 288;
		out_x       = 720;
		out_y       = 288;
	//----------------------------------
		ht       = 864;
		hbp      = 139;
		vt       = 625;
		vbp      = 22;
		vspw     = 2;
		hspw     = 2;  
	}
	if(mode==DISP_TV_MOD_NTSC)
	{	
		b_interlace   = 1;
		src_x       = 720;
		src_y       = 240;
		scl_x       = 720;
		scl_y       = 240;
		out_x       = 720;
		out_y       = 240;
	//----------------------------------
		ht       = 858;
		hbp      = 118;
		vt       = 525;
		vbp      = 18;
		vspw     = 2;
		hspw     = 2; 
	}		
  //使能TCON1模块 
	vblank_len = vt/2 - src_y - 2;
	if(vblank_len >= 32)
	{
		start_delay	= 30;
	}
	else
	{
		start_delay	= vblank_len - 2;//23 modify//old:cfg->start_delay	= vblank_len - 1
	}
  write32(TCON_Base_Address+0x090,read32(TCON_Base_Address+0x090)|((u64_t)0x1 << 31));		
	write32(TCON_Base_Address+0x090,read32(TCON_Base_Address+0x090)|0);//0=DE 2=BLUE[1V电压] TCON0与TCON1同一源时将关闭LCD输出
	write32(TCON_Base_Address+0x090,read32(TCON_Base_Address+0x090)|((start_delay & 0x1f) << 4) );
	if(b_interlace  == 1)write32(TCON_Base_Address+0x090,read32(TCON_Base_Address+0x090)| (0x1 << 20) );	
	//源宽高
	write32(TCON_Base_Address+0x094,(src_x-1) << 16 | (src_y-1) << 0);
	write32(TCON_Base_Address+0x098,(scl_x-1) << 16 | (scl_y-1) << 0);		
	write32(TCON_Base_Address+0x09c,(out_x-1) << 16 | (out_y-1) << 0);//输出宽高
  //
	write32(TCON_Base_Address+0x0a0, ((ht - 1) << 16)   | ((hbp - 1) << 0));
	write32(TCON_Base_Address+0x0a4, ((vt) << 16)       | ((vbp - 1) << 0));
	write32(TCON_Base_Address+0x0a8, ((hspw - 1) << 16) | ((vspw - 1) << 0));	
  //	
	write32(TCON_Base_Address+0x0f0, 0);
	write32(TCON_Base_Address+0x0f4, 0x0fffffff);
	return 0;
}
/*——————————————————————————————————————————————————————————————————————————————
TVE配置
*/
int TVE_Config(__u8 mode)
{
	TVE_set_reg_base(0x01C0A000);	
	TVE_Clock_Open();//打开时钟
  TVE_set_tv_mode(0,mode);
	TVE_init();
	TVE_dac_autocheck_enable(0);//dac检测使能
	TVE_dac_enable(0);	
//	tve_low_enhance(0,0);
#if 0
	TVE_SET_BIT(TVE_004,0x1<<8);//输出条色	
#endif	
  TVE_open(0);
	return 0;
}
/*——————————————————————————————————————————————————————————————————————————————
TV初始化
*/
void TVE_Init(void)
{
	sysprintf("TVE Init\r\n");	
	TVE_Clock_Open();	
#ifdef LCD_TYPE_TV_PAL_720_576	
	TCON1_Init(DISP_TV_MOD_PAL);	
	TVE_Config(DISP_TV_MOD_PAL);	 
#endif
#ifdef LCD_TYPE_TV_NTSC_720_480
	TCON1_Init(DISP_TV_MOD_NTSC);	 
	TVE_Config(DISP_TV_MOD_NTSC);		
#endif

}


