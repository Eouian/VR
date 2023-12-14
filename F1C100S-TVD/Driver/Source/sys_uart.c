#include "sys_uart.h"
#include "sys_gpio.h"
#include "sys_types.h"
#include <string.h>
#include <stdio.h>
#include "sys_io.h"
#include "sys_interrupt.h"
#include "sys_delay.h"

#define vaArg(list, type) ((type *)(list += sizeof(type)))[-1]
#define vaStart(list, param) list = (INT8*)((INT)&param + sizeof(param))

//默认控制参数-可UART初始化前配置
__align(0x4) uart_ctrl_data uart_cd[3]=
{
	{UART_DATA_8BIT,UART_STOP_1BIT,0,0},//UART0
	{UART_DATA_8BIT,UART_STOP_1BIT,0,0},//UART1
	{UART_DATA_8BIT,UART_STOP_1BIT,0,0} //UART2
};
 
/*---------------------------------------------------
串口0中断
----------------------------------------------------*/
void UART0_ISR(void)
{
	u32_t addr=UART0_ADDR;
	u32 s=read32(addr+ 0x08);
	if((s&0x4)==0x4)//接收中断
	{
		char val=read32(addr + 0x00);
		sysprintf("R0=%02x\r\n",val);
		Sys_Uart_Put(UART0,val);//发送测试-发送完成产生发送中断
	}
	if((s&0x2)==0x2)//发送中断
	{
		sysprintf("T0.\r\n");
	}
}
/*---------------------------------------------------
串口1中断
----------------------------------------------------*/
void UART1_ISR(void)
{
	u32_t addr=UART1_ADDR;
	u32 s=read32(addr+ 0x08);
	if((s&0x4)==0x4)//接收中断
	{
		char val=read32(addr + 0x00);
		sysprintf("R1=%02x\r\n",val);
		Sys_Uart_Put(UART1,val);//发送测试-发送完成产生发送中断
	}
	if((s&0x2)==0x2)//发送中断
	{
		sysprintf("T1.\r\n");
	}
}
/*---------------------------------------------------
串口2中断
----------------------------------------------------*/
void UART2_ISR(void)
{
	u32_t addr=UART2_ADDR;
	u32 s=read32(addr+ 0x08);
	if((s&0x4)==0x4)//接收中断
	{
		char val=read32(addr + 0x00);
		sysprintf("R2=%02x\r\n",val);
		Sys_Uart_Put(UART2,val);//发送测试-发送完成产生发送中断
	}
	if((s&0x2)==0x2)//发送中断
	{
		sysprintf("T2.\r\n");
	}
}
//返回UART寄存器首地址
unsigned int uart_get_addr(enum eUart uart)
{
u32 addr;	
  switch(uart)
	{
		case UART0:
			addr = UART0_ADDR;
			break;
		case UART1:
			addr = UART1_ADDR;
			break;
		case UART2:
			addr = UART2_ADDR;		
			break;		
	}	
	return addr;
}
/*
串口初始化
uart 串口号
cpu_frequency cpu频率
Baudrate 波特率
Interrupt 中断使能
IntMode 中断模式
pvNewISR 中断函数
*/
void Sys_Uart_Init_F(enum eUart uart,unsigned int Baudrate,int Interrupt,int IntMode,void* pvNewISR)
{
	u32_t addr;
	u32_t val,Div,APBclk;
	u8 LW;
	
  APBclk=get_apb_frequency();
	Div=APBclk/Baudrate/16; //{波特率=时钟(APBCLK)/(16*除数)}
	
	//UART引脚配置
  switch(uart)
	{
		case UART0:
			addr = UART0_ADDR;
		  LW=20;
			// 配置输出脚
#ifdef UART0_PF2_PF4_EN	//PF2.4
			GPIO_Congif(GPIOF,GPIO_Pin_2,GPIO_Mode_011,GPIO_PuPd_NOPULL);//RX
			GPIO_Congif(GPIOF,GPIO_Pin_4,GPIO_Mode_011,GPIO_PuPd_NOPULL);//TX		
#else //PE0.1		
			GPIO_Congif(GPIOE,GPIO_Pin_0,GPIO_Mode_101,GPIO_PuPd_NOPULL);//RX
			GPIO_Congif(GPIOE,GPIO_Pin_1,GPIO_Mode_101,GPIO_PuPd_NOPULL);//TX
#endif
			break;
		case UART1:
			addr = UART1_ADDR;
		  LW=21;
			// 配置输出脚
#ifdef	UART1_PD3_PD4_EN //PD3.4
			GPIO_Congif(GPIOD,GPIO_Pin_3,GPIO_Mode_011,GPIO_PuPd_NOPULL);//RX
			GPIO_Congif(GPIOD,GPIO_Pin_4,GPIO_Mode_011,GPIO_PuPd_NOPULL);//TX		
#else	//PA2.3
			GPIO_Congif(GPIOA,GPIO_Pin_2,GPIO_Mode_101,GPIO_PuPd_NOPULL);//RX
			GPIO_Congif(GPIOA,GPIO_Pin_3,GPIO_Mode_101,GPIO_PuPd_NOPULL);//TX		
#endif
			break;
		case UART2:
			addr = UART2_ADDR;		
		  LW=22;
			// 配置输出脚
#ifdef 	UART2_PD13_PD14_EN //PD13.14
			GPIO_Congif(GPIOD,GPIO_Pin_13,GPIO_Mode_011,GPIO_PuPd_NOPULL);//TX
			GPIO_Congif(GPIOD,GPIO_Pin_14,GPIO_Mode_011,GPIO_PuPd_NOPULL);//RX		
#else	//PE7.8
			GPIO_Congif(GPIOE,GPIO_Pin_7,GPIO_Mode_011,GPIO_PuPd_NOPULL);//TX
			GPIO_Congif(GPIOE,GPIO_Pin_8,GPIO_Mode_011,GPIO_PuPd_NOPULL);//RX		
#endif	
			break;		
	}
	/*时钟与复位*/
	S_BIT(0x01c20068,LW);
	C_BIT(0x01c202d0,LW);
	delay_us(100);
	S_BIT(0x01c202d0,LW);
	/*中断配置*/
	write32(addr + 0x04,IntMode);
	/*配置默认FIFO*/
	write32(addr + 0x08, (0x0<<6)|(0x3<<4)|(0x7<<0));//默认单字节触发，2分之1发送触发 
	/*模块控制*/
	write32(addr + 0x10, 0x0);
	/*选中除数寄存器*/
	val = read32(addr + 0x0c); 
	val |= (1 << 7);
	write32(addr + 0x0c, val);
	/*波特率除数设置*/
	write32(addr + 0x04, (Div >> 8) & 0xff);//注意需要先写高位-再写低位，否则一些优化场合会出错。	
	write32(addr + 0x00, Div & 0xff);
	/*取消选中除数寄存器*/
	val = read32(addr + 0x0c);
	val &= ~(1 << 7);
	write32(addr + 0x0c, val);
	/*写控制位*/
	val = read32(addr + 0x0c);
	val &= ~0x1f;
	val |= (uart_cd[uart].data_len << 0) //数据位：0: 5 bits	1: 6 bits	2: 7 bits	3: 8 bits
			| (uart_cd[uart].stop_bit << 2) 		//停止位  0: 1 stop bit 1: 1.5 stop bits when DLS (LCR[1:0]) is zero, else 2 stop bit
			| (uart_cd[uart].Parity_Enable << 3)  //校验位  0关，1开
		  | (uart_cd[uart].Even_Parit_Select << 4); //奇偶位  0: Odd Parity 1: Even Parity 2/3: Reverse LCR[4]
	write32(addr + 0x0c, val);
	
	//使能中断
	if(Interrupt==1)
	{
		if(pvNewISR==0)//默认中断函数
		{
			if(uart==UART0)IRQ_Init(IRQ_LEVEL_1,IRQ_UART0,UART0_ISR,3);
			else if(uart==UART1)IRQ_Init(IRQ_LEVEL_1,IRQ_UART1,UART1_ISR ,3);
			else if(uart==UART2)IRQ_Init(IRQ_LEVEL_1,IRQ_UART2,UART2_ISR ,3);
		}else//自定义中断函数
    {
			if(uart==UART0)IRQ_Init(IRQ_LEVEL_1,IRQ_UART0,pvNewISR,3);
			else if(uart==UART1)IRQ_Init(IRQ_LEVEL_1,IRQ_UART1,pvNewISR,3);
			else if(uart==UART2)IRQ_Init(IRQ_LEVEL_1,IRQ_UART2,pvNewISR,3);
		}		
	}
}
/*读取接收FIFO计数*/
int Sys_Uart_Read_RxFifo_count(enum eUart uart)
{
u32 addr=uart_get_addr(uart);	
	return read32(addr + 0x84);
}
/*读取发送FIFO计数*/
int Sys_Uart_Read_TxFifo_count(enum eUart uart)
{
u32 addr=uart_get_addr(uart);	
	return read32(addr + 0x80);
}
/*设置接收FIFO计数
0: 1 character in the FIFO 单字节中断模式
1: FIFO ¼ full
2: FIFO ½ full
3: FIFO-2 less than full
超时接收中断使用时不能设置为0
*/
void Sys_Uart_Set_Rx_Fifo_Trigger(enum eUart uart,int Trigger)
{
  u32 addr=uart_get_addr(uart);	
	u32 val=read32(addr + 0x08);
	val&=~(0x3<<6);
	val|=(Trigger<<6);
	write32(addr + 0x08,val); 
}
/*设置发送FIFO计数
0: FIFO empty
1: 2 characters in the FIFO
2: FIFO ¼ full
3: FIFO ½ full
*/
void Sys_Uart_Set_Tx_Fifo_Trigger(enum eUart uart,int Trigger)
{
  u32 addr=uart_get_addr(uart);	
	u32 val=read32(addr + 0x08);
	val&=~(0x3<<4);
	val|=(Trigger<<4);
	write32(addr + 0x08,val); 
}
/*
设置发送中断开关
en =0  en =1 开
*/
void Sys_Uart_TX_Interrupt(enum eUart uart,int en)
{
u32 addr=uart_get_addr(uart);		
  if(en)
	{
		S_BIT(addr + 0x04,1);
	}
	else
	{		
		C_BIT(addr + 0x04,1);
	}
}
/*
串口输出字符
*/
void Sys_Uart_Put(enum eUart uart,char c)
{
u32 addr;	
  switch(uart)
	{
		case UART0:
			addr = UART0_ADDR;
			break;
		case UART1:
			addr = UART1_ADDR;
			break;
		case UART2:
			addr = UART2_ADDR;		
			break;		
	}	
	while((read32(addr + 0x7c) & (0x1 << 1)) == 0);
	write32(addr + 0x0, c);
}

/*-----------------------------打印相关----------------------------------*/
/*
设置调试口
*/
int UART_DBG=0;
void Sys_SET_UART_DBG(enum eUart uart)
{
	UART_DBG = uart;
}
/*
输出一个字
*/
void Sys_Uart_Putc(char c)
{
	Sys_Uart_Put((enum eUart)UART_DBG,c);
}

VOID sysPutString(INT8 *string)
{
		while (*string != '\0')
		{
			Sys_Uart_Putc(*string);
			string++;
		}

}

static VOID sysPutRepChar(INT8 c, INT count)
{
	while (count--)
	Sys_Uart_Putc(c);
}


static VOID sysPutStringReverse(INT8 *s, INT index)
{
	while ((index--) > 0)
	Sys_Uart_Putc(s[index]);
}

static VOID sysPutNumber(INT value, INT radix, INT width, INT8 fill)
{
	INT8    buffer[40];
	INT     bi = 0;
	UINT32  uvalue;
	UINT16  digit;
	UINT16  left = FALSE;
	UINT16  negative = FALSE;

	if (fill == 0)
	    	fill = ' ';

	if (width < 0)
	{
		width = -width;
		left = TRUE;
	}

	if (width < 0 || width > 80)
	    	width = 0;

	if (radix < 0)
	{
		radix = -radix;
		if (value < 0)
		{
			negative = TRUE;
			value = -value;
	    	}
	}

	uvalue = value;

	do
	{
		if (radix != 16)
		{
			digit = uvalue % radix;
			uvalue = uvalue / radix;
		}
		else
		{
			digit = uvalue & 0xf;
			uvalue = uvalue >> 4;
		}
		buffer[bi] = digit + ((digit <= 9) ? '0' : ('A' - 10));
		bi++;

		if (uvalue != 0)
		{
			if ((radix == 10)
			    && ((bi == 3) || (bi == 7) || (bi == 11) | (bi == 15)))
			{
				buffer[bi++] = ',';
			}
		}
	}
	while (uvalue != 0);

	if (negative)
	{
		buffer[bi] = '-';
		bi += 1;
	}

	if (width <= bi)
		sysPutStringReverse(buffer, bi);
	else
	{
		width -= bi;
		if (!left)
			sysPutRepChar(fill, width);
		sysPutStringReverse(buffer, bi);
		if (left)
		    	sysPutRepChar(fill, width);
	}
}


static INT8 *FormatItem(INT8 *f, INT a)
{
	INT8   c;
	INT    fieldwidth = 0;
	INT    leftjust = FALSE;
	INT    radix = 0;
	INT8   fill = ' ';

	if (*f == '0')
		fill = '0';

	while ((c = *f++) != 0)
	{
		if (c >= '0' && c <= '9')
		{
			fieldwidth = (fieldwidth * 10) + (c - '0');
		}
		else
			switch (c)
			{
				case '\000':
					return (--f);
				case '%':
				    	Sys_Uart_Putc('%');
				    	return (f);
				case '-':
				    	leftjust = TRUE;
				    	break;
				case 'c':
				{
				        if (leftjust)
				        	Sys_Uart_Putc(a & 0x7f);

				        if (fieldwidth > 0)
				            	sysPutRepChar(fill, fieldwidth - 1);

				        if (!leftjust)
				            	Sys_Uart_Putc(a & 0x7f);
				        return (f);
				}
				case 's':
				{
				        if (leftjust)
				        	sysPutString((PINT8)a);

				        if (fieldwidth > strlen((PINT8)a))
				            	sysPutRepChar(fill, fieldwidth - strlen((PINT8)a));

				        if (!leftjust)
				           	sysPutString((PINT8)a);
				        return (f);
				}
				case 'd':
				case 'i':
				   	 radix = -10;
				break;
				case 'u':
				    	radix = 10;
				break;
				case 'x':
				    	radix = 16;
				break;
				case 'X':
				    	radix = 16;
				break;
				case 'o':
				    	radix = 8;
				break;
				default:
				    	radix = 3;
				break;      /* unknown switch! */
			}
		if (radix)
		    break;
	}

	if (leftjust)
	    	fieldwidth = -fieldwidth;

	sysPutNumber(a, radix, fieldwidth, fill);

	return (f);
}


void sysprintf(char * pcStr,...)
{
	char *argP;
  if(get_uart_clk_state((enum eUart)UART_DBG)==0)return; //判断串口是否打开
	vaStart(argP, pcStr);       /* point at the end of the format string */
	while (*pcStr)
	{                       /* this works because args are all ints */
		if (*pcStr == '%')
					pcStr = FormatItem(pcStr + 1, vaArg(argP, INT));
		else
					Sys_Uart_Putc(*pcStr++);
	}
}
//最后发送回车符
void sysprintf_r(char * pcStr,...)
{
	char *argP;
  if(get_uart_clk_state((enum eUart)UART_DBG)==0)return; //判断串口是否打开
	vaStart(argP, pcStr);       /* point at the end of the format string */
	while (*pcStr)
	{                       /* this works because args are all ints */
		if (*pcStr == '%')
					pcStr = FormatItem(pcStr + 1, vaArg(argP, INT));
		else
					Sys_Uart_Putc(*pcStr++);
	}
	Sys_Uart_Putc('\r');
}
//最后发送回车换行符
void sysprintf_rn(char * pcStr,...)
{
	char *argP;
  if(get_uart_clk_state((enum eUart)UART_DBG)==0)return; //判断串口是否打开
	vaStart(argP, pcStr);       /* point at the end of the format string */
	while (*pcStr)
	{                       /* this works because args are all ints */
		if (*pcStr == '%')
					pcStr = FormatItem(pcStr + 1, vaArg(argP, INT));
		else
					Sys_Uart_Putc(*pcStr++);
	}
	Sys_Uart_Putc('\r');
	Sys_Uart_Putc('\n');
}

void printf_null(char * pcStr,...)
{

}




