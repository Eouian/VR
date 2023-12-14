#ifndef _SYS_BMP_H_
#define _SYS_BMP_H_




void LCD_Draw_Picture(int x,int y,int w,int h,unsigned char *pic);
int LCD_Draw_Picture_2(int x,int y,unsigned char *buff);
void LCD_Clear(int x,int y,int w,int h,int color);
void show_mini_photo(void);
int save_photo(void);
#endif
