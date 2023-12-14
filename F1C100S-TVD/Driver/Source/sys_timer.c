#include "sys_timer.h"
#include "sys_interrupt.h"
#include "sys_uart.h"
#include "sys_gpio.h"
#include "sys_delay.h"
#include "sys_io.h"


/*
定时器初始化
timer=TIMER0,TIMER1,TIMER2【f1c100s有三个定时器】
IRQ_EN=中断使能
time_us=中断时间 us 
*/
void Timer_Init(u8 timer,u32_t time_us,u8 IRQ_EN)
{
int val=0;
	write32(TIMER_CTRL(timer),0);
	//Timer 模式 0连续 1单
	val=0;
	write32(TIMER_CTRL(timer),read32(TIMER_CTRL(timer))	|	((val)<<7));
	//Timer 分频
	val=0;
	write32(TIMER_CTRL(timer),read32(TIMER_CTRL(timer))	|	((val)<<4));
	//Timer 时钟源
	val=1; //24Mhz
	write32(TIMER_CTRL(timer),read32(TIMER_CTRL(timer))	|	((val)<<2));	
	//Timer 重装计数值
	val=1; //=1重装计数值
	write32(TIMER_CTRL(timer),read32(TIMER_CTRL(timer))	|	((val)<<1));	

	//Timer 计数值
	val=24*time_us;
	write32(TIMER_INTV(timer)	,val);
	
	//使能中断
	if(IRQ_EN==1)
	{
		write32(TIMER_IRQ_EN,read32(TIMER_IRQ_EN)	|	(1<<timer));
	}
}
/*
定时器开
*/
void Timer_enable(u8 timer)
{
	write32(TIMER_CTRL(timer),read32(TIMER_CTRL(timer))	|	((1)<<0));
}
/*
定时器关
*/
void Timer_disable(u8 timer)
{
	write32(TIMER_CTRL(timer),read32(TIMER_CTRL(timer)) & (~((1)<<0)) );
}
/*
定时器中断
*/
unsigned int sys_count_timer=0;
void TIMER0_ISR(void)
{
	//清中断
	Clear_interrupt(TIMER0);
}
void TIMER1_ISR(void)
{
	Clear_interrupt(TIMER1);//清中断
  //	
	sys_count_timer++;
	if(sys_count_timer>=0xffffffff)sys_count_timer=0;	
}
void TIMER2_ISR(void)
{
	Clear_interrupt(TIMER2);//清中断
}
/*
读定时器计数
*/
unsigned int Read_time_ms(void)
{
	return sys_count_timer;
}

/*
清计数值
*/
void Reset_time(void)
{
	sys_count_timer=0;
}
/*
延时函数(定时器)
*/
void Tdelay_ms(int ms)
{
	unsigned int time=0;
	time=sys_count_timer;
	while((sys_count_timer-time)<ms);
}
/*
定时器初始化
T=计数值
IRQ=1 开总中断
*/
void Sys_TimerX_Init(u8 timer,int T,Sys_timer sys_tiemr,int IRQ)
{
	Timer_Init(timer,T,1);	
	IRQ_Init(IRQ_LEVEL_1,(INT_SOURCE_E)(IRQ_Timer0+timer),sys_tiemr,3);	
	Timer_enable(timer);
  if(IRQ)sysSetLocalInterrupt(ENABLE_IRQ);//开IRQ中断
}

/*
定时器测试
*/
void Timer_Demo(void)
{
  Sys_TimerX_Init(TIMER1,1000,TIMER1_ISR,1);
	while(1)
	{
		sysprintf("TIMER %d \r\n",Read_time_ms());
		delay_ms(1000);
	}
}



//-------------------------------------------------------------------------------
//----------------------------音视频Timer----------------------------------------
//-------------------------------------------------------------------------------
/*音视频同步时间初始化-
us计数一次*/
void AVS_Time_Init(int AVS_TIME_Inx,int us)
{
	AVS_Time_Init_x(AVS_TIME_Inx,24*us);		
}
/*音视频同步时间初始化
c为时钟分频值，时钟24M，c=24时为1us计数一次*/
void AVS_Time_Init_x(int AVS_TIME_Inx,int c)
{
  /*24M时钟通过*/
  S_BIT(0x01C20000+0x0144,31);
  int reg=read32(F1C100S_TIMER_BASE+0x8c);
  if(AVS_TIME_Inx==AVS_TIME_0)
  {
    reg&=~((u64_t)0xffff<<0);
    reg|=(c-1)<<0;
  }
  else if(AVS_TIME_Inx==AVS_TIME_1)
  {
    reg&=~((u64_t)0xffff<<16);
    reg|=(c-1)<<16; 
  }
  /*初始为us/次*/
  write32(F1C100S_TIMER_BASE+0x8c,reg);			
}

/*停止计数器*/
void AVS_Time_Stop(int AVS_TIME_Inx)
{
	if(AVS_TIME_Inx==AVS_TIME_0)
	{
		write32(F1C100S_TIMER_BASE+0x84,0);	
		write32(F1C100S_TIMER_BASE+0x80,read32(F1C100S_TIMER_BASE+0x80) &(~ ((1)<<0)));	
	}
    else if(AVS_TIME_Inx==AVS_TIME_1)
	{
		write32(F1C100S_TIMER_BASE+0x88,0);
		write32(F1C100S_TIMER_BASE+0x80,read32(F1C100S_TIMER_BASE+0x80) &(~ ((1)<<1)));
	}	
}
/*开始并计数器清0*/
void AVS_Time_Start(int AVS_TIME_Inx)
{
	if(AVS_TIME_Inx==AVS_TIME_0)
	{
		write32(F1C100S_TIMER_BASE+0x84,0);	
		write32(F1C100S_TIMER_BASE+0x80,read32(F1C100S_TIMER_BASE+0x80) | (1)<<0);	
	}
    else if(AVS_TIME_Inx==AVS_TIME_1)
	{
		write32(F1C100S_TIMER_BASE+0x88,0);
		write32(F1C100S_TIMER_BASE+0x80,read32(F1C100S_TIMER_BASE+0x80) | (1)<<1);
	}	
}
/*读计数值 33位计数器读出的为高32位*/
unsigned int AVS_Time_Read(int AVS_TIME_Inx)
{
	if(AVS_TIME_Inx==AVS_TIME_0)return read32(F1C100S_TIMER_BASE+0x84)*2;
    else if(AVS_TIME_Inx==AVS_TIME_1)return read32(F1C100S_TIMER_BASE+0x88)*2;	
	return 0;
}
/*计数暂停*/
void AVS_Time_Pause(int AVS_TIME_Inx)
{
	if(AVS_TIME_Inx==AVS_TIME_0)write32(F1C100S_TIMER_BASE+0x80,read32(F1C100S_TIMER_BASE+0x80) | (1)<<8);
    else if(AVS_TIME_Inx==AVS_TIME_1)write32(F1C100S_TIMER_BASE+0x80,read32(F1C100S_TIMER_BASE+0x80) | (1)<<9);	
}
/*计数继续*/
void AVS_Time_Recover(int AVS_TIME_Inx)
{
	if(AVS_TIME_Inx==AVS_TIME_0)write32(F1C100S_TIMER_BASE+0x80,read32(F1C100S_TIMER_BASE+0x80) &(~ ((1)<<8)));
    else if(AVS_TIME_Inx==AVS_TIME_1)write32(F1C100S_TIMER_BASE+0x80,read32(F1C100S_TIMER_BASE+0x80) &(~ ((1)<<9)));	
}
