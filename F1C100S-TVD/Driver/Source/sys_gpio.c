#include "sys_gpio.h"
#include "sys_io.h"
#include "reg_ccu.h"



/*
GPIO 初始化
*/
void GPIO_Config
	(
		GPIO_TypeDef * GPIOx, //GPIO
		unsigned int GPIO_Pin, //Pin
		GPIOMode_TypeDef GPIOMode,//模式
		GPIOPuPd_TypeDef GPIO_PuPd//上下拉
	)
{ 	
	GPIOx->CFG[GPIO_Pin/8] &=~  (0xF<<(GPIO_Pin%8*4));//清0
	
	if(GPIOMode==GPIO_Mode_IN)       GPIOx->CFG[GPIO_Pin/8] &= ~ (0x1<<(GPIO_Pin%8*4));//输入模式		
	else if(GPIOMode==GPIO_Mode_OUT) GPIOx->CFG[GPIO_Pin/8] |=   (0x1<<(GPIO_Pin%8*4));//输出模式		
  else                             GPIOx->CFG[GPIO_Pin/8] |=   (GPIOMode<<(GPIO_Pin%8*4));//其它模式	
	
	//上下拉
	GPIOx->PUL[GPIO_Pin/16]&= ~ (0x3<<(GPIO_Pin%16*2));//清0
	GPIOx->PUL[GPIO_Pin/16]|=  (GPIO_PuPd<<(GPIO_Pin%16*2));//写入	
}

/*
GPIO输出1
*/
void GPIO_SET
	(
		GPIO_TypeDef * GPIOx, //GPIO
		unsigned int GPIO_Pin //Pin
	)
{ 
	GPIOx->DAT |=   (0x1<<GPIO_Pin);
}
/*
GPIO输出0
*/
void GPIO_RESET
	(
		GPIO_TypeDef * GPIOx, //GPIO
		unsigned int GPIO_Pin //Pin
	)
{ 
	GPIOx->DAT &= ~ (0x1<<GPIO_Pin);
}
/*
GPIO读 返回0-1
*/
u8 GPIO_READ 
	(
		GPIO_TypeDef * GPIOx, //GPIO
		unsigned int GPIO_Pin //Pin
	)
{ 
	if(GPIOx->DAT&((1)<<GPIO_Pin))return 1;
	else return 0;
}
/*
GPIO驱动能力设置
Multi_Driving=0-3，0最低，3最高
*/
void GPIO_Multi_Driving 
	(
		GPIO_TypeDef * GPIOx, //GPIO
		unsigned int GPIO_Pin, //Pin
		unsigned int Multi_driving //驱动能力	
	)
{ 
	if(Multi_driving>3)Multi_driving=3;
	GPIOx->DRV[GPIO_Pin/16]&= ~ (0x3<<(GPIO_Pin%16*2));//清0
	GPIOx->DRV[GPIO_Pin/16]|=  (Multi_driving<<(GPIO_Pin%16*2));//写入	
}
/*
GPIO中断配置
*/
#define PIO_Base_Address (0x01C20800)
void GPIO_External_Inerrupt_Config(int GPIOi,unsigned int GPIO_Pin,int mode)
{
	unsigned int n=GPIO_Pin/8;
	unsigned int y=GPIO_Pin%8;
	unsigned int addr=PIO_Base_Address+0x200+(GPIOi*0x20)+(n*0x4);
	
	// External Interrupt Configure Register 
	write32(addr,read32(addr)&(~(0xf<<(y*4))));//清0
	write32(addr,read32(addr)|(mode<<(y*4)));//写入
//	sysprintf("GPIO_%08x...\r\n",addr);
//	sysprintf("GPIO_%08x...\r\n",read32(addr));
}
/*
GPIO中断使能
*/
void GPIO_External_Inerrupt_Enable(int GPIOi,unsigned int GPIO_Pin)
{
	unsigned int addr=PIO_Base_Address+0x200+(GPIOi*0x20)+0x10;
	
	//External Interrupt Control Register
	S_BIT(addr,GPIO_Pin);
//	sysprintf("GPIO_%08x...\r\n",addr);
//	sysprintf("GPIO_%08x...\r\n",read32(addr));
}
/*
GPIO中断失能
*/
void GPIO_External_Inerrupt_Disable(int GPIOi,unsigned int GPIO_Pin)
{
	unsigned int addr=PIO_Base_Address+0x200+(GPIOi*0x20)+0x10;
	
	//External Interrupt Control Register
	C_BIT(addr,GPIO_Pin);
}
/*
GPIO中断Status
*/
unsigned int GPIO_External_Inerrupt_Status(int GPIOi)
{
	unsigned int addr=PIO_Base_Address+0x200+(GPIOi*0x20)+0x14;
	unsigned int val=read32(addr);
	write32(addr,val);//Write ‘1’ to clear
	return val;
}


