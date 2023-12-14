#include "sys_clock.h"
#include "sys_gpio.h"
#include "sys_delay.h"
#include "sys_lcd.h"
#include "sys_cache.h"
#include "sys_interrupt.h"
#include "sys_timer.h"
#include <string.h>
#include <stdio.h>
#include <sys_mmu.h>
#include "sys_io.h"
#include "sys_tvd.h"
#include <stdlib.h>
#include "sys_uart.h"
#include "sys_defe.h"
//--------------
#include "sys_lcd_conf.h"
//--------------
#include "mconf.h"
/*---------------------------------------------------
说明：
  1."sys_lcd_conf.h"中选择LCD分辩率;
  2.输入摄像头只支持标准PAL(720x576),NTSC(720x480);
	3.开发板JP2跳线帽选择TVIN0输入还是TVIN1输入,JP7跳线帽选择SO1输入还是SO2输入;
2020/07/02
  1.加入800x480LCD支持;
	2.TVD加入多缓存，解决显示画面撕裂问题;
2020/08/04
  1.加入800x480截图支持;
----------------------------------------------------*/


__align(1024) unsigned int LCDbuff1[XSIZE_PHYS*YSIZE_PHYS];
__align(1024) unsigned int LCDbuff2[XSIZE_PHYS*YSIZE_PHYS];
static int CSItime=0,defetime =0 ;
static volatile __de_scal_dev_t *scal_dev[1];

int main(void)
{ 
	int CPU_Frequency_hz=408*1000000;	
	int mode=TVD_SOURCE_NTSC;	
	Sys_Clock_Init(CPU_Frequency_hz);
	sys_mmu_init();		
	Sys_Timer1_Init();

	Sys_LCD_Init(XSIZE_PHYS,YSIZE_PHYS,(unsigned int *)LCDbuff1,(unsigned int *)LCDbuff2);	
	f1c100s_tvd_Init(0,mode);
	Set_Layer_format(0,0xA,5);//设置层0为ARGB8888
	set_video_window(0,0,XSIZE_PHYS,YSIZE_PHYS);
	Enable_Layer_Video();
	
	
	while(1)
	{	
		
	}
}

