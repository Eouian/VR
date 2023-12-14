#ifndef __SYS_UART_H__
#define __SYS_UART_H__
#include "sys_clock.h"
#include "sys_io.h"
enum eUart
{
  UART0=0,
	UART1,
	UART2
};

#define UART_ERBFI  (1<<0)//0Bit-接收数据与超时中断,可判断接收超时
#define UART_ETBEI  (1<<1)//1Bit-发送FIFO空中断

//数据位：0: 5 bits	1: 6 bits	2: 7 bits	3: 8 bits
#define UART_DATA_8BIT  3  
#define UART_DATA_7BIT  2
#define UART_DATA_6BIT  1
#define UART_DATA_5BIT  0
//停止位  0: 1 stop bit 1: 1.5 stop bits when DLS (LCR[1:0]) is zero, else 2 stop bit	
#define UART_STOP_1BIT      0
#define UART_STOP_1d5_2BIT  1
//奇偶位  0: Odd Parity 1: Even Parity 2/3: Reverse LCR[4]
#define UART_Odd_Parity     0
#define UART_Even_Parity    1

typedef struct{
 int data_len;//数据位
 int stop_bit;//停止位
 int Parity_Enable;//校验位  0关，1开
 int Even_Parit_Select;//奇偶位
}uart_ctrl_data;

extern uart_ctrl_data uart_cd[3];

#define UART0_ADDR 0x01c25000
#define UART1_ADDR 0x01C25400
#define UART2_ADDR 0x01c25800

#define get_uart_clk_state(UARTx) (read32(0x01c20068)&(1 << (20+UARTx)))/*返回串口时钟是否打开*/	

void Sys_Uart_Init_F(enum eUart uart,unsigned int Baudrate,int Interrupt,int IntMode,void* pvNewISR);

#define Sys_Uart_Init_X(uart,cpu_frequency,Baudrate,Interrupt,IntMode,pvNewISR) Sys_Uart_Init_F(uart,Baudrate,Interrupt,IntMode,pvNewISR)
#define Sys_Uart_Init(uart,cpu_frequency,Baudrate,Interrupt) Sys_Uart_Init_F(uart,Baudrate,Interrupt,UART_ERBFI|UART_ETBEI,0)
#define Sys_Uart_Init_C(uart,Baudrate) Sys_Uart_Init_F(uart,Baudrate,0,UART_ERBFI|UART_ETBEI,0)

void Sys_Uart_Put(enum eUart uart,char c);
void sysprintf(char * pcStr,...);
void Sys_SET_UART_DBG(enum eUart uart);
void printf_null(char * pcStr,...);
#define sysprintf_null printf_null
#define Sys_Uart0_Init(x) Sys_Uart_Init(UART0,get_cpu_frequency(),x,0)
#define sys_Uart0_Init(a,x) Sys_Uart_Init(UART0,a,x,0)
void sysprintf_r(char * pcStr,...);
void sysprintf_rn(char * pcStr,...);
void Sys_Uart_Putc(char c);	
void Sys_Uart_TX_Interrupt(enum eUart uart,int en);
int Sys_Uart_Read_RxFifo_count(enum eUart uart);
int Sys_Uart_Read_TxFifo_count(enum eUart uart);
void Sys_Uart_Set_Rx_Fifo_Trigger(enum eUart uart,int Trigger);
void Sys_Uart_Set_Tx_Fifo_Trigger(enum eUart uart,int Trigger);
#endif
