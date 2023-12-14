#ifndef _MCONF_H_
#define _MCONF_H_
//--------------
#include "sys_lcd_conf.h"
//--------------in.h"

#if (defined LCD_TYPE_RGB43_480_272)||(defined LCD_TYPE_RGB43_800_480)
	#define LCD_TYPE_RGB43_480_272 /*定义窗口显示,未定义全屏显示*/
#else 
	#define XV_W	XSIZE_PHYS	//窗口显示时图像的宽与高
	#define XV_H	YSIZE_PHYS
	#define XV_POS_X	0	//窗口显示时图像的位置
	#define XV_POS_Y	0
#endif

//-------------------------------------------
#ifdef	LCD_TYPE_RGB43_480_272
  #ifdef GUIx 
		#define XV_W	366	//窗口显示时图像的宽与高
		#define XV_H	200
		#define XV_POS_X	58	//窗口显示时图像的位置
		#define XV_POS_Y	36
	#else
		#define XV_W	XSIZE_PHYS	//窗口显示时图像的宽与高
		#define XV_H	YSIZE_PHYS
		#define XV_POS_X	0	//窗口显示时图像的位置
		#define XV_POS_Y	0
	#endif
#endif	
//-------------------------------------------
#ifdef	LCD_TYPE_RGB43_800_480
  #ifdef GUIx 
		#define XV_W	610	//窗口显示时图像的宽与高
		#define XV_H	408
		#define XV_POS_X	95	//窗口显示时图像的位置
		#define XV_POS_Y	36
	#else
		#define XV_W	XSIZE_PHYS	//窗口显示时图像的宽与高
		#define XV_H	YSIZE_PHYS
		#define XV_POS_X	0	//窗口显示时图像的位置
		#define XV_POS_Y	0
	#endif
#endif

#endif
