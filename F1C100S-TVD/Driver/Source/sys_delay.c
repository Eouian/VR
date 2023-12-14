#include "sys_delay.h"
#include "sys_timer.h"

//使用AV定时器来做延时处理
#define AVS_TIME_x AVS_TIME_1
/*
延时--毫秒
*/
void delay_ms(int ms)
{
  AVS_Time_Init(AVS_TIME_x,10);//10us计时一次
  AVS_Time_Start(AVS_TIME_x);
	ms*=100;
  while(1)
  {
    if(AVS_Time_Read(AVS_TIME_x)>=ms)break;
  }
}
/*
延时--微秒
*/
void delay_us(int us)
{
  AVS_Time_Init_x(AVS_TIME_x,1);//(1/24)us计时一次
  AVS_Time_Start(AVS_TIME_x);
	us*=24;
  while(1)
  {
    if(AVS_Time_Read(AVS_TIME_x)>=us)break;
  }
}

