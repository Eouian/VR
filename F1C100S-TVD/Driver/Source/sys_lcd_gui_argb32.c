#include "sys_lcd_gui_argb32.h"
#include "sys_lcd.h"
#include "reg-debe.h"
#include "reg-tcon.h"
#include "sys_interrupt.h"
#include "sys_uart.h"
#include "sys_delay.h"
#include "sys_io.h"
#include <stdlib.h>
//--------------
#include "sys_lcd_conf.h"
//--------------
#include "font.h"





extern unsigned int LCDbuff2[XSIZE_PHYS*YSIZE_PHYS*2];
unsigned int *BT1=(unsigned int *)LCDbuff2;

/*
清LCD到指定色
*/
void LCD_Clear_ARGB32(unsigned int Color)
{
int i;
	for(i=0;i<(XSIZE_PHYS*YSIZE_PHYS);i++)BT1[i]=Color;
}
/*
LCD 画点
*/
void LCD_Draw_Points_ARGB32(int x,int y,unsigned int Color)
{
	BT1[y*XSIZE_PHYS+x]=Color;
}
/*
LCD 画矩形
*/
void LCD_Draw_Rectangle_ARGB32(int x,int y,int w,int h,unsigned int Color)
{
int i,l;
	for(i=0;i<h;i++)//y
	for(l=0;l<w;l++)//x
	BT1[(y+i)*XSIZE_PHYS+(x+l)]=Color;
}

/*
显示ASCII
fx =1反显
fd =放大倍数
*/
void LCD_Draw_ASCII_ARGB32(int x,int y,int n,unsigned int Color1,unsigned int Color2,int fx,int fd)
{
u8 i,l,d;
u8 w=8*fd;	
u8 h=16*fd;	
	
	for(i=0;i<h;i+=fd)//y
	{
		d=ASCII_1608[n-0x20][i/fd];
		for(l=0;l<w;l+=fd)//x
		{			
			if(d&0x80)LCD_Draw_Rectangle_ARGB32(x+l,y+i,fd,fd,Color1);
			else
			{
				if(fx==1)//反显
				{
					LCD_Draw_Rectangle_ARGB32(x+l,y+i,fd,fd,Color2);
				}
			}
			d=d<<1;	
		}
	}
}
/*
显示String
fx =1反显
fd =放大倍数
*/
void LCD_Draw_String_ARGB32(int x,int y,char *str,unsigned int Color1,unsigned int Color2,int fx,int fd)
{
	while(*str!='\0')
	{
		LCD_Draw_ASCII_ARGB32(x,y,*str,Color1,Color2,fx,fd);
		x+=(8*fd);
		if(x>(XSIZE_PHYS-(8*fd))){y++;x=0;}
		str++;
	}
}

/*
显示16色位图片
*/
void LCD_Draw_Picture_ARGB32(int x,int y,int w,int h,char *pic)
{
int i,l,a,b;
short *tpic=(short *)pic;			
	for(i=0;i<h;i++)//y
	{
		a=(y+i)*XSIZE_PHYS+x;
		b=w*i;
		for(l=0;l<w;l++)BT1[a+l]=tpic[b+l];
	}
}

/*
读16色位显存 
*/
void LCD_Read_Lcdbuff_ARGB32(int x,int y,int w,int h,char *pic)
{
int i,l,a,b;
short *tpic=(short *)pic;			
	for(i=0;i<h;i++)//y
	{
		a=(y+i)*XSIZE_PHYS+x;
		b=w*i;
		for(l=0;l<w;l++)tpic[b+l]=BT1[a+l];
	}
}



/*
显示汉字
fx =1反显
fd =放大倍数
*/
void LCD_Draw_HZ_ARGB32(int x,int y,int v,unsigned int Color1,unsigned int Color2,int fx,int fd,char * hzprt)
{
u8 i,l,s,d;
u8 w=8*fd;	
u8 h=v*fd;	
u8 e=v/8;	
	for(i=0;i<h;i+=fd)//y
	{
		for(s=0;s<e;s++)
		{
			d=hzprt[(i*e)/fd+s];
			for(l=0;l<w;l+=fd)//x
			{			
				if(d&0x80)LCD_Draw_Rectangle_ARGB32((s*w)+x+l,y+i,fd,fd,Color1);
				else
				{
					if(fx==1)//反显
					{
						LCD_Draw_Rectangle_ARGB32((s*w)+x+l,y+i,fd,fd,Color2);
					}
				}
				d=d<<1;	
			}
	  }
	}
}
void LCD_Draw_HZ1616_ARGB32(int x,int y,int n,unsigned int Color1,unsigned int Color2,int fx,int fd)
	{LCD_Draw_HZ_ARGB32(x,y,16,Color1,Color2,fx,fd,(char *)HZ_1616[n]);}
void LCD_Draw_HZ2424_ARGB32(int x,int y,int n,unsigned int Color1,unsigned int Color2,int fx,int fd)
	{LCD_Draw_HZ_ARGB32(x,y,24,Color1,Color2,fx,fd,(char *)HZ_2424[n]);}
void LCD_Draw_HZ3232_ARGB32(int x,int y,int n,unsigned int Color1,unsigned int Color2,int fx,int fd)
	{LCD_Draw_HZ_ARGB32(x,y,32,Color1,Color2,fx,fd,(char *)HZ_3232[n]);}
void LCD_Draw_HZ4040_ARGB32(int x,int y,int n,unsigned int Color1,unsigned int Color2,int fx,int fd)
	{LCD_Draw_HZ_ARGB32(x,y,40,Color1,Color2,fx,fd,(char *)HZ_4040[n]);}
void LCD_Draw_HZ4848_ARGB32(int x,int y,int n,unsigned int Color1,unsigned int Color2,int fx,int fd)
	{LCD_Draw_HZ_ARGB32(x,y,48,Color1,Color2,fx,fd,(char *)HZ_4848[n]);}

	
	
	
	