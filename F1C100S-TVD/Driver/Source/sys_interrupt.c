#include "sys_types.h"
#include "sys_uart.h"
#include "sys_interrupt.h"
#include "sys_delay.h"
#include "sys_cache.h"
#include "sys_gpio.h"
#include "sys_io.h"


BOOL volatile _sys_bIsAICInitial = FALSE;/* 中断系统初始化标志 */

/*寄存器写入偏移值*/
#define AIC_REG_OFFSET    						    			0x4
#define AICRegRead(RegBase, RegNum)							read32(RegBase+RegNum*AIC_REG_OFFSET)
#define AICRegWrite(RegBase, RegNum, Value)			write32(RegBase+RegNum*AIC_REG_OFFSET, Value)


typedef void (*sys_pvFunPtr)(); /* 函数指针类型定义 */
VOID F1C100S_Interrupt_Shell(){}     /* 空函数*/
__align(0x4) 	
sys_pvFunPtr sysIrqHandlerTable[] = { 0,                /* 0 */
                                  F1C100S_Interrupt_Shell,	/* 1 */
                                  F1C100S_Interrupt_Shell,	/* 2 */
                                  F1C100S_Interrupt_Shell,	/* 3 */
                                  F1C100S_Interrupt_Shell,	/* 4 */
                                  F1C100S_Interrupt_Shell,	/* 5 */
                                  F1C100S_Interrupt_Shell,	/* 6 */
                                  F1C100S_Interrupt_Shell,	/* 7 */
                                  F1C100S_Interrupt_Shell,	/* 8 */
                                  F1C100S_Interrupt_Shell,	/* 9 */
                                  F1C100S_Interrupt_Shell,	/* 10 */
                                  F1C100S_Interrupt_Shell,	/* 11 */
                                  F1C100S_Interrupt_Shell,	/* 12 */
                                  F1C100S_Interrupt_Shell,	/* 13 */
                                  F1C100S_Interrupt_Shell,	/* 14 */
                                  F1C100S_Interrupt_Shell,	/* 15 */
                                  F1C100S_Interrupt_Shell,	/* 16 */
                                  F1C100S_Interrupt_Shell,	/* 17 */
                                  F1C100S_Interrupt_Shell,	/* 18 */
                                  F1C100S_Interrupt_Shell,	/* 19 */
                                  F1C100S_Interrupt_Shell,	/* 20 */
                                  F1C100S_Interrupt_Shell,	/* 21 */
                                  F1C100S_Interrupt_Shell,	/* 22 */
                                  F1C100S_Interrupt_Shell,	/* 23 */
                                  F1C100S_Interrupt_Shell,	/* 24 */
                                  F1C100S_Interrupt_Shell,	/* 25 */
                                  F1C100S_Interrupt_Shell,	/* 26 */
                                  F1C100S_Interrupt_Shell,	/* 27 */
                                  F1C100S_Interrupt_Shell,	/* 28 */
                                  F1C100S_Interrupt_Shell,	/* 29 */
                                  F1C100S_Interrupt_Shell,	/* 30 */
                                  F1C100S_Interrupt_Shell,	/* 31 */
                                  F1C100S_Interrupt_Shell,	/* 32 */
                                  F1C100S_Interrupt_Shell,	/* 33 */
                                  F1C100S_Interrupt_Shell,	/* 34 */
                                  F1C100S_Interrupt_Shell,	/* 35 */
                                  F1C100S_Interrupt_Shell,	/* 36 */
                                  F1C100S_Interrupt_Shell,	/* 37 */
                                  F1C100S_Interrupt_Shell,	/* 38 */
                                  F1C100S_Interrupt_Shell,	/* 39 */
                                  F1C100S_Interrupt_Shell 	/* 40 */
 
                                };

sys_pvFunPtr sysFiqHandlerTable[] = { 0,
                                  F1C100S_Interrupt_Shell,	/* 1 */
                                  F1C100S_Interrupt_Shell,	/* 2 */
                                  F1C100S_Interrupt_Shell,	/* 3 */
                                  F1C100S_Interrupt_Shell,	/* 4 */
                                  F1C100S_Interrupt_Shell,	/* 5 */
                                  F1C100S_Interrupt_Shell,	/* 6 */
                                  F1C100S_Interrupt_Shell,	/* 7 */
                                  F1C100S_Interrupt_Shell,	/* 8 */
                                  F1C100S_Interrupt_Shell,	/* 9 */
                                  F1C100S_Interrupt_Shell,	/* 10 */
                                  F1C100S_Interrupt_Shell,	/* 11 */
                                  F1C100S_Interrupt_Shell,	/* 12 */
                                  F1C100S_Interrupt_Shell,	/* 13 */
                                  F1C100S_Interrupt_Shell,	/* 14 */
                                  F1C100S_Interrupt_Shell,	/* 15 */
                                  F1C100S_Interrupt_Shell,	/* 16 */
                                  F1C100S_Interrupt_Shell,	/* 17 */
                                  F1C100S_Interrupt_Shell,	/* 18 */
                                  F1C100S_Interrupt_Shell,	/* 19 */
                                  F1C100S_Interrupt_Shell,	/* 20 */
                                  F1C100S_Interrupt_Shell,	/* 21 */
                                  F1C100S_Interrupt_Shell,	/* 22 */
                                  F1C100S_Interrupt_Shell,	/* 23 */
                                  F1C100S_Interrupt_Shell,	/* 24 */
                                  F1C100S_Interrupt_Shell,	/* 25 */
                                  F1C100S_Interrupt_Shell,	/* 26 */
                                  F1C100S_Interrupt_Shell,	/* 27 */
                                  F1C100S_Interrupt_Shell,	/* 28 */
                                  F1C100S_Interrupt_Shell,	/* 29 */
                                  F1C100S_Interrupt_Shell,	/* 30 */
                                  F1C100S_Interrupt_Shell,	/* 31 */
                                  F1C100S_Interrupt_Shell,	/* 32 */
                                  F1C100S_Interrupt_Shell,	/* 33 */
                                  F1C100S_Interrupt_Shell,	/* 34 */
                                  F1C100S_Interrupt_Shell,	/* 35 */
                                  F1C100S_Interrupt_Shell,	/* 36 */
                                  F1C100S_Interrupt_Shell,	/* 37 */
                                  F1C100S_Interrupt_Shell,	/* 38 */
                                  F1C100S_Interrupt_Shell,	/* 39 */
                                  F1C100S_Interrupt_Shell 	/* 40 */		
                                };

																
/*
IRQ中断进入															
*/
void IRQ_Handler2(void)
{	
	//读出中断函数地址与计算出中断号
	int VectorAddress=read32(INTC_Base_Address);//向量地址先读出
	/*跳转到中断函数*/
	(*(sys_pvFunPtr)(*(u32 *)VectorAddress))();
} 

void SWI_Handler2(void){while(1);}
															
/*
设置中断
nIntTypeLevel 模式等级
eIntNo 中断号
PVOID pvNewISR 中断函数
Priority 0-3 0最低 3最高
*/
PVOID IRQ_Init(INT32 nIntTypeLevel, INT_SOURCE_E eIntNo, PVOID pvNewISR , CHAR Priority)
{
	PVOID  _mOldVect;
	UINT32 _mRegValue;
	/*判断中断系统有没有初始化*/
	if (!_sys_bIsAICInitial)
	{
		_sys_bIsAICInitial = TRUE;
	  write32(INTC_Base_Address+0x4,(u32)(sysIrqHandlerTable));//写基地址
	}
  /*设置中断函数*/
	switch (nIntTypeLevel)
	{
		case FIQ_LEVEL_0:
			_mOldVect = (PVOID) sysFiqHandlerTable[eIntNo];
			sysFiqHandlerTable[eIntNo] = (sys_pvFunPtr)pvNewISR;
		break;

		case IRQ_LEVEL_1:
		case IRQ_LEVEL_2:
		case IRQ_LEVEL_3:
		case IRQ_LEVEL_4:
		case IRQ_LEVEL_5:
		case IRQ_LEVEL_6:
		case IRQ_LEVEL_7:
		  	 _mOldVect = (PVOID) sysIrqHandlerTable[eIntNo];
		   	sysIrqHandlerTable[eIntNo] = (sys_pvFunPtr)pvNewISR;
		break;

		default:
		   ;
	}	
	
	/*设置优先级*/
	_mRegValue = AICRegRead(INTC_Base_Address+0x60, (eIntNo)/16 );//读出优先级;	
	_mRegValue = _mRegValue & (~((0x3)<<(((eIntNo)%16)*2)));//清0
	_mRegValue = _mRegValue | ((Priority)<<(((eIntNo)%16)*2));//写值
	AICRegWrite(INTC_Base_Address+0x60, ((eIntNo)/16), _mRegValue );//写入优先级

	
  /*开中断*/	
	_mRegValue = AICRegRead(INTC_Base_Address+0x20, eIntNo/32 );//读出中断使能;	
	_mRegValue = _mRegValue | ((0x1)<<(eIntNo%32));
	AICRegWrite(INTC_Base_Address+0x20, (eIntNo/32), _mRegValue );//写入中断使能
	
	return _mOldVect;	
}																
		
/*
设置中断
nIntTypeLevel 模式等级
eIntNo 中断号
PVOID pvNewISR 中断函数
Priority 0-3 0最低 3最高
*/
PVOID __IRQ_Init(INT32 nIntTypeLevel, INT_SOURCE_E eIntNo, PVOID pvNewISR , CHAR Priority, CHAR en)
{
	PVOID  _mOldVect;
	UINT32 _mRegValue;
	/*判断中断系统有没有初始化*/
	if (!_sys_bIsAICInitial)
	{
		_sys_bIsAICInitial = TRUE;
	  write32(INTC_Base_Address+0x4,(u32)(sysIrqHandlerTable));//写基地址
	}
  /*设置中断函数*/
	switch (nIntTypeLevel)
	{
		case FIQ_LEVEL_0:
			_mOldVect = (PVOID) sysFiqHandlerTable[eIntNo];
			sysFiqHandlerTable[eIntNo] = (sys_pvFunPtr)pvNewISR;
		break;

		case IRQ_LEVEL_1:
		case IRQ_LEVEL_2:
		case IRQ_LEVEL_3:
		case IRQ_LEVEL_4:
		case IRQ_LEVEL_5:
		case IRQ_LEVEL_6:
		case IRQ_LEVEL_7:
		  	 _mOldVect = (PVOID) sysIrqHandlerTable[eIntNo];
		   	sysIrqHandlerTable[eIntNo] = (sys_pvFunPtr)pvNewISR;
		break;

		default:
		   ;
	}	
	
	/*设置优先级*/
	_mRegValue = AICRegRead(INTC_Base_Address+0x60, (eIntNo)/16 );//读出优先级;	
	_mRegValue = _mRegValue & (~((0x3)<<(((eIntNo)%16)*2)));//清0
	_mRegValue = _mRegValue | ((Priority)<<(((eIntNo)%16)*2));//写值
	AICRegWrite(INTC_Base_Address+0x60, ((eIntNo)/16), _mRegValue );//写入优先级

	
  /*开中断*/	
	_mRegValue = AICRegRead(INTC_Base_Address+0x20, eIntNo/32 );//读出中断使能;	
	_mRegValue = _mRegValue | ((en)<<(eIntNo%32));
	AICRegWrite(INTC_Base_Address+0x20, (eIntNo/32), _mRegValue );//写入中断使能
	
	return _mOldVect;	
}	
/*强制中断
使能强制中断后要记得在中断函数里清0强制位，不然会一直进中断造成死机现象
*/	
int FastForcing (INT_SOURCE_E eIntNo,int en)
{
	UINT32 _mRegValue;
  /*开中断*/	
	_mRegValue = AICRegRead(INTC_Base_Address+0x50, eIntNo/32 );//读出中断使能;	
	if(en)_mRegValue = _mRegValue | ((1)<<(eIntNo%32));
	else  _mRegValue &= ~(_mRegValue | ((1)<<(eIntNo%32)));
	AICRegWrite(INTC_Base_Address+0x50, (eIntNo/32), _mRegValue );//写入中断使能

	return 0;	
}
/*使能中断*/	
PVOID IRQ_Enable(INT_SOURCE_E eIntNo)
{
	PVOID  _mOldVect;
	UINT32 _mRegValue;
  /*开中断*/	
	_mRegValue = AICRegRead(INTC_Base_Address+0x20, eIntNo/32 );//读出中断使能;	
	_mRegValue = _mRegValue | ((0x1)<<(eIntNo%32));
	AICRegWrite(INTC_Base_Address+0x20, (eIntNo/32), _mRegValue );//写入中断使能
	
	return _mOldVect;	
}	
/*失能中断*/	
PVOID IRQ_Disable(INT_SOURCE_E eIntNo)
{
	PVOID  _mOldVect;
	UINT32 _mRegValue;
  /*开中断*/	
	_mRegValue = AICRegRead(INTC_Base_Address+0x20, eIntNo/32 );//读出中断使能;	
	_mRegValue = _mRegValue&(~((0x1)<<(eIntNo%32)));
	AICRegWrite(INTC_Base_Address+0x20, (eIntNo/32), _mRegValue );//写入中断使能
	
	return _mOldVect;	
}	
/*设置启用CPSR中断*/															
INT32 sysSetLocalInterrupt(INT32 nIntState)
{
	INT32 temp;

	switch (nIntState)
	{
		case ENABLE_IRQ:
		case ENABLE_FIQ:
		case ENABLE_FIQ_IRQ:
			__asm
			{
			   MRS    temp, CPSR
			   AND    temp, temp, nIntState
			   MSR    CPSR_c, temp
			}
		break;

		case DISABLE_IRQ:
		case DISABLE_FIQ:
		case DISABLE_FIQ_IRQ:
			__asm
			{
			   MRS    temp, CPSR
			   ORR    temp, temp, nIntState
			   MSR    CPSR_c, temp
			}
		   break;

		default:
		   	;
	}
	return 0;
}	

/*读取CPSR的I位状态*/																	
BOOL sysGetIBitState(void)
{
	INT32 temp;

	__asm
	{
		MRS	temp, CPSR
	}

	if (temp & 0x80)
		return FALSE;
	else
		return TRUE;
}																
																
																
																
																
																
																
																
																
																
																
																
																
																
