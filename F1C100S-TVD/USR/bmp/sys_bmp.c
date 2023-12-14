#include "sys_uart.h"
#include "sys_delay.h"
#include "sys_types.h"
#include "string.h"
#include "sys_bmp.h"
//--------------
#include "sys_lcd_conf.h"
//--------------
#include <stdlib.h>
#include "sys_cache.h"
#include "sys_tvd.h"

extern unsigned int LCDbuff1[];
extern unsigned int LCDbuff2[];

typedef struct tagBITMAPFILEHEADER {
//  unsigned short bfType;   // 19778，必须是BM字符串，对应的十六进制为0x4d42,十进制为19778
  unsigned int bfSize;    // 文件大小
  unsigned short bfReserved1; // 一般为0
  unsigned short bfReserved2; // 一般为0
  unsigned int bfOffBits;   // 从文件头到像素数据的偏移，也就是这两
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
  unsigned int biSize;    // 此结构体的大小
  int biWidth;        // 图像的宽
  int biHeight;        // 图像的高
  unsigned short biPlanes;  // 1
  unsigned short biBitCount; // 一像素所占的位数，一般为24
  unsigned int biCompression; // 0
  unsigned int biSizeImage;  // 像素数据所占大小, 这个值应该等于上面文件头结构中bfSize-bfOffBits
  int biXPelsPerMeter;    // 0
  int biYPelsPerMeter;    // 0
  unsigned int biClrUsed;   // 0 
  unsigned int biClrImportant;// 0
} BITMAPINFOHEADER;
/*
显示32色位图片
*/
void LCD_Draw_Picture(int x,int y,int w,int h,unsigned char *pic)
{
int i,l,a,b;
u32 *tpic=(u32 *)pic;			
	for(i=0;i<h;i++)//y
	{
		a=(y+i)*XSIZE_PHYS+x;
		b=w*i;
		for(l=0;l<w;l++)LCDbuff2[a+l]=tpic[b+l];
	}
}
/*
显示32色位图片
*/
void LCD_Draw_Picture1(int x,int y,int w,int h,unsigned char *pic)
{
int i,l,a,b;

	for(i=0;i<h;i++)//y
	{
		a=(y+i)*XSIZE_PHYS+x;
		b=w*i;
		for(l=0;l<w;l++)LCDbuff2[a+l]=pic[(b+l)*4+0]<<0 | pic[(b+l)*4+1]<<8 | pic[(b+l)*4+2]<<16 | pic[(b+l)*4+3]<<24;
	}
}
/*
读bmp+显示32色位图片
*/
int LCD_Draw_Picture_2(int x,int y,unsigned char *buff)
{
int res=1,i=0;
__align(4) unsigned char buff0[12];	
__align(4) unsigned char buff1[40];	
char * databuff;	
int w,h;
BITMAPFILEHEADER  *BitMapFileHeader;
BITMAPINFOHEADER	*BitMapInfoHeader;
		if(res==1)
		{
			if((buff[0]==0x42)&(buff[1]==0x4D))
			{
				
				for(i=0;i<12;i++)buff0[i]=buff[2+i];//读文件头
				for(i=0;i<40;i++)buff1[i]=buff[14+i];//读文件信息
				
				
				BitMapFileHeader=(BITMAPFILEHEADER *)(&buff0[0]);
				BitMapInfoHeader=(BITMAPINFOHEADER *)(&buff1[0]);
				sysprintf("bmp file size: %d \r\n",BitMapFileHeader->bfSize);
				sysprintf("bmp data bfOffBits: %d \r\n",BitMapFileHeader->bfOffBits);
				sysprintf("bmp w: %d \r\n",BitMapInfoHeader->biWidth);
				sysprintf("bmp h: %d \r\n",BitMapInfoHeader->biHeight);
				sysprintf("bmp biBit: %d \r\n",BitMapInfoHeader->biBitCount);
				if(BitMapInfoHeader->biBitCount!=32)
				{
					sysprintf("bmp biBit err!%d \r\n");
					return -1;
				}
        w=BitMapInfoHeader->biWidth;
				h=BitMapInfoHeader->biHeight;
				
				//显示
				//LCD_Draw_Picture(x,y,w,abs(h),&buff[BitMapFileHeader->bfOffBits]);
				
				LCD_Draw_Picture1(x,y,w,abs(h),&buff[BitMapFileHeader->bfOffBits]);
				
			}else
			{
				sysprintf("bmp file type err! no BM type!\r\n");
				return -1;
			}
		}	
	return 0;
}

//void bmpsetH(unsigned char *bitmap,int h,unsigned char *bitmap2)
//{
//int i,j,k,bitmapH1,bitmapH2,bitmapV1,bitmapV2,bitmapSize1,bitmapSize2,bitmapLen1,bitmapLen2;
//int startH1,startH2,R,G,B,R1,G1,B1;
//int redress1;
//int bitmapNH1;
//int redress2;
//int bitmapNH2;
//double Hval;
//double Hvala;
//int Hvalb,Hvalc;
//	
////  位图宽1 ＝ 取字节集数据 (位图数据, #整数型, 19)
//  bitmapH1=*((int*)(bitmap+18));
//  bitmapV1=*((int*)(bitmap+22));
//  redress1=bitmapH1%4;
//  bitmapNH1=bitmapH1*3+redress1;
//  bitmapLen1=bitmapNH1*bitmapV1;
//  bitmapSize1=bitmapLen1+54;
//  redress2=h%4;
//  bitmapNH2=h*3+redress2;
//  bitmapLen2=bitmapNH2*bitmapV1;
//  bitmapSize2=bitmapLen2+54;
//  memcpy(bitmap2,bitmap,54);
//  memcpy(bitmap2+2,&bitmapSize2,4);
//  memcpy(bitmap2+18,&h,4);
//  Hval=(double)bitmapH1/h;
//  Hvala=0;
//  Hvalb=0,Hvalc=0;
//  for (i=1;i<=bitmapV1;i++)
//  {
//  startH2=bitmapSize2-i*bitmapNH2;
//  startH1=bitmapSize1-i*bitmapNH1;
//    B1=bitmap[startH1];
//  G1=bitmap[startH1+1];
//  R1=bitmap[startH1+2];
//  for(j=1;j<=h;j++)
//  {
//  startH2=startH2+3;
//      Hvala=Hvala+Hval;
//  if (Hvala<1)
//  {
//       Hvalb=3;
//  }
//  else
//  {
//  Hvalc=Hvala;
//  Hvalb=Hvalc*3;
//  }
//  startH1=bitmapSize1-i*bitmapNH1+Hvalb;
//  B=(bitmap[startH1-3]+B1)/2;
//  G=(bitmap[startH1-2]+G1)/2;
//  R=(bitmap[startH1-1]+R1)/2;
//  bitmap2[startH2-3]=B;
//  bitmap2[startH2-2]=G;
//  bitmap2[startH2-1]=R;
//  B1=bitmap[startH1-3];
//  G1=bitmap[startH1-2];
//  R1=bitmap[startH1-1];
// 
//  }
//   Hvala=0;
//  }
// 
//}

//void BmpSetV(unsigned char *bitmap,int v,unsigned char *bitmap2)
//{  
// 
//double Vval,Vvala=0;
//  int i,j,Vvalb=0,bitH1,bitNH1,bitV1,redress,bitLen1,bitsize1,bitLen2,bitsize2;
//int start3=0,start2=0,start1=0,G1,B1,R1,R,G,B;
//  bitH1=*((int*)(bitmap+18));
//bitV1=*((int*)(bitmap+22));
//redress=bitH1%4;
//bitNH1=bitH1*3+redress;
//bitLen1=bitNH1*bitV1;
//bitsize1=bitLen1+54;
//bitLen2=bitNH1*v;
//bitsize2=bitLen2+54;
//memcpy(bitmap2,bitmap,54);
//memcpy(bitmap2+2,&bitsize2,4);
//memcpy(bitmap2+22,&v,4);
//Vval=bitV1/(double)v;
//for (i=1;i<=bitH1;i++)
//{
//     start3=bitsize1-bitNH1+i*3;
// B1=bitmap[start3-3];
// G1=bitmap[start3-2];
// R1=bitmap[start3-1];
// for (j=1;j<=v;j++)
// {
// Vvala=Vvala+Vval;
// if (Vvala<1)Vvalb=1;
// else Vvalb=Vvala;
// start2=bitsize2-j*bitNH1+i*3;
// start1=bitsize1-Vvalb*bitNH1+i*3;
// if (Vvalb<bitV1)
// {
// start3=bitsize1-Vvalb*bitNH1-bitNH1+i*3;
// if(i==1)
// {
//          bitmap2[start2-3]=bitmap[start1-3];
//  bitmap2[start2-2]=bitmap[start1-2];
//bitmap2[start2-1]=bitmap[start1-1];
// }
// else
// {
// B=(B1+bitmap[start1-3])/2;
// G=(G1+bitmap[start1-2])/2;
// R=(R1+bitmap[start1-1])/2;
// bitmap2[start2-3]=B;
// bitmap2[start2-2]=G;
// bitmap2[start2-1]=R;
// B1=bitmap[start3-3];
// G1=bitmap[start3-2];
// R1=bitmap[start3-1];
// }
// }
// else
// {
//          bitmap2[start2-3]=bitmap[start1-3];
//          bitmap2[start2-2]=bitmap[start1-2];
//bitmap2[start2-1]=bitmap[start1-1];
// }
// 
// 
// }
// Vvala=0;
//}
// 
//}
/*------------------------------------------------------
混合平面YUV422转rgb888
*/
__inline void YUV422_TO_RGB888(u8 Y,u8 U,u8 V,u8* R,u8* G,u8* B)
{
int r,g,b;
	r = (int)((298*(int)Y + 411* (int)V - 57344)>>8);
	g = (int)((298*(int)Y - 101* (int)U - 211* (int)V+ 34739)>>8);
	b = (int)((298*(int)Y + 519* (int)U- 71117)>>8);
	
	if(r<0)r=0;	if(r>255)r=255;
	if(g<0)g=0;	if(g>255)g=255;
	if(b<0)b=0;	if(b>255)b=255;	
	
	*R=(u8)r;
	*G=(u8)g;
	*B=(u8)b;
}
/*------------------------------------------------------
YUV422图像转ARGB图像
*/
void bmp_yuv422_to_rgb8888(u8* b_y,u8* b_c,u8* bmp,int w,int h)
{
int i=0,len=w*h;
	for(i=0;i<len;i+=2)
	{
		YUV422_TO_RGB888(b_y[i+0],b_c[i+0],b_c[i+1],&bmp[(i+0)*4+2],&bmp[(i+0)*4+1],&bmp[(i+0)*4+0]);  bmp[(i+0)*4+3]=0xff;
		YUV422_TO_RGB888(b_y[i+1],b_c[i+0],b_c[i+1],&bmp[(i+1)*4+2],&bmp[(i+1)*4+1],&bmp[(i+1)*4+0]);  bmp[(i+1)*4+3]=0xff;
	}
}
/*------------------------------------------------------
打印数组
*/
void sprintf_buf(char *name,unsigned char *buff,int len)
{
int i;
	sysprintf("%s\r\n",name);
	for(i=0;i<len;i+=1)
	{
		if((i>0)&&(i%16==0))sysprintf("\r\n");		
		if(i%16==0)sysprintf("[%04X] ",i);
		sysprintf("%02X,",buff[i]);
	}
  sysprintf("\r\n"); 
}
/*------------------------------------------------------
图像缩放2
*/
void FiResize2(u8* src, u8* dst, int srcWidth,int srcHeight,int dstWidth, int dstHeight,int channels) 
{
int step1 = srcWidth*channels; 
int step2 =dstWidth*channels; 
float kx,ky;
long xrIntFloat_16=((srcWidth)<<16)/dstWidth+1; 
long yrIntFloat_16=((srcHeight)<<16)/dstHeight+1; 
const long csDErrorX=-(1<<15)+(xrIntFloat_16>>1); 
const long csDErrorY=-(1<<15)+(yrIntFloat_16>>1);
long srcy_16=csDErrorY;
long kq,kq1,srcx_16,kp,kp1;
unsigned long u_8,v_8,pm3_16,pm2_16,pm1_16,pm0_16;
int i,j,k;
	for (j = 0; j < dstHeight; j++) 
	{ 
		kq=srcy_16>>16; 
		kq =kq>=0?kq:0; 
		kq1 = kq<srcHeight-1?kq+1:kq;
		srcx_16=csDErrorX; 
		for (i = 0;i < dstWidth; i++) 
		{
			kp=srcx_16>>16; 
			kp =kp>=0?kp:0; 
			kp1 = kp<srcWidth-1?kp+1:kp; 
			u_8=(srcx_16 & 0xFFFF)>>8; 
			v_8=(srcy_16 & 0xFFFF)>>8; 
			pm3_16=(u_8*v_8); 
			pm2_16=(u_8*(unsigned long)(256-v_8)); 
			pm1_16=(v_8*(unsigned long)(256-u_8)); 
			pm0_16=((256-u_8)*(256-v_8));
			for (k =0;k<channels;k++) 
			{ 
				dst[j*step2+i*channels+k] = (pm0_16*src[kq*step1+kp*channels+k] +pm2_16*src[kq*step1+kp1*channels+k]+ pm1_16*src[kq1*step1+kp*channels+k] 
				+pm3_16*src[kq1*step1+kp1*channels+k])>>16 ; 
			} 
			srcx_16+=xrIntFloat_16; 
		} 
		srcy_16+=yrIntFloat_16;
	}
}
/*------------------------------------------------------
保存图片并显示全屏照片
*/
extern int SAVE_BMP;//保存图像标志 1=开始保存;
extern unsigned char buff_y[720*576];
extern unsigned char buff_c[720*576];
__align(4) u8 bmp[720*576*4];//保存bmp图像
__align(4) u8 bmp1[XSIZE_PHYS*YSIZE_PHYS*4];//缩放后的图像
int save_photo(void)
{
int i=0;
	SAVE_BMP=1;	
	while(i<40)
	{
		i++;
		if(SAVE_BMP==0)break;//等待帧完成
		delay_ms(1);
	}
//	if(i>=40)return -1;
//	sysInvalidCache();
	sysFlushCache(D_CACHE);//
	bmp_yuv422_to_rgb8888(buff_y,buff_c,bmp,TVD_OUT_W,TVD_OUT_H);//YUV422转ARGB8888
	FiResize2(bmp,bmp1,TVD_OUT_W,TVD_OUT_H,XSIZE_PHYS,YSIZE_PHYS,4);//缩放 
	LCD_Draw_Picture(0,0,XSIZE_PHYS,YSIZE_PHYS,bmp1);//显示
	return 0;
}
/*------------------------------------------------------
显示左下角小照片
*/
void show_mini_photo(void)
{
#if (defined LCD_TYPE_TV_NTSC_720_480)||(defined LCD_TYPE_TV_PAL_720_576)
	FiResize2(bmp,bmp1,720,480,100,100,4); 
	LCD_Draw_Picture(XSIZE_PHYS-100-65,YSIZE_PHYS-100-40,100,100,bmp1);
#else	
	#ifdef LCD_TYPE_RGB43_480_272
		FiResize2(bmp,bmp1,TVD_OUT_W,TVD_OUT_H,33,33,4); 	
		LCD_Draw_Picture(XSIZE_PHYS-33-13,YSIZE_PHYS-33-15,33,33,bmp1);
	#endif
	#ifdef LCD_TYPE_RGB43_800_480
		FiResize2(bmp,bmp1,TVD_OUT_W,TVD_OUT_H,55,57,4); 	
		LCD_Draw_Picture(XSIZE_PHYS-65-13,YSIZE_PHYS-65-20,55,57,bmp1);
	#endif
#endif
	BSP_TVD_capture_on2();	
}


/*
清屏
*/
void LCD_Clear(int x,int y,int w,int h,int color)
{
int i,l,a,b;

	for(i=0;i<h;i++)//y
	{
		a=(y+i)*XSIZE_PHYS+x;
		b=w*i;
		for(l=0;l<w;l++)LCDbuff2[a+l]=color;
	}
}







