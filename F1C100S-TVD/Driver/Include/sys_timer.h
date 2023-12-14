#ifndef __SYS_TIME_H__
#define __SYS_TIME_H__

#include "sys_types.h"
#include "sys_io.h"

#define TIMER0 0
#define TIMER1 1
#define TIMER2 2

#define F1C100S_TIMER_BASE	(0x01C20C00)
#define TIMER_IRQ_EN				F1C100S_TIMER_BASE+(0x00)
#define TIMER_IRQ_STA				F1C100S_TIMER_BASE+(0x04)
#define TIMER_CTRL(x)				F1C100S_TIMER_BASE+((x + 1) * 0x10 + 0x00)
#define TIMER_INTV(x)				F1C100S_TIMER_BASE+((x + 1) * 0x10 + 0x04)
#define TIMER_CUR(x)				F1C100S_TIMER_BASE+((x + 1) * 0x10 + 0x08)


/*音视频同步时间初始化-100us*/
#define AVS_TIME_0 		0
#define AVS_TIME_1 		1
typedef void (*Sys_timer)(); /* 函数指针类型定义 */

void AVS_Time_Init_x(int AVS_TIME_Inx,int c);
void AVS_Time_Init(int AVS_TIME_Inx,int us);
/*开始并计数器清0*/
void AVS_Time_Start(int AVS_TIME_Inx);
void AVS_Time_Stop(int AVS_TIME_Inx);
/*读计数值*/
unsigned int AVS_Time_Read(int AVS_TIME_Inx);
/*计数暂停*/
void AVS_Time_Pause(int AVS_TIME_Inx);
/*计数继续*/
void AVS_Time_Recover(int AVS_TIME_Inx);

void TIMER0_ISR(void);
void Timer_Demo(void);
void Tdelay_ms(int ms);
void Timer_enable(u8 timer);
void Timer_disable(u8 timer);
void Timer_Init(u8 timer,u32_t time_us,u8 IRQ_EN);
unsigned int Read_time_ms(void);
#define get_time_ms Read_time_ms
void Sys_Timer1_Init(void);
void Sys_TimerX_Init(u8 timer,int T,Sys_timer sys_tiemr,int IRQ);
//	
#define Clear_interrupt(_timer) write32(TIMER_IRQ_STA,read32(TIMER_IRQ_STA)	| (1<<_timer))	
//
void TIMER0_ISR(void);
void TIMER1_ISR(void);	
void TIMER2_ISR(void);
//
#define Sys_Timer0_Init() 	Sys_TimerX_Init(TIMER0,1000,TIMER0_ISR,1)
#define Sys_Timer1_Init() 	Sys_TimerX_Init(TIMER1,1000,TIMER1_ISR,1)
#define Sys_Timer2_Init() 	Sys_TimerX_Init(TIMER2,1000,TIMER2_ISR,1)
#define Timer0_Init() 		Sys_TimerX_Init(TIMER0,1000,TIMER0_ISR,1)
#define Timer1_Init() 		Sys_TimerX_Init(TIMER1,1000,TIMER1_ISR,1)
#define Timer2_Init() 		Sys_TimerX_Init(TIMER2,1000,TIMER2_ISR,1)
//

#endif
