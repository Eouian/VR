#ifndef __IO_H__
#define __IO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "sys_types.h"

static   u8_t read8(virtual_addr_t addr)
{
	return ( *((volatile u8_t *)(addr)) );
}

static   u16_t read16(virtual_addr_t addr)
{
	return ( *((volatile u16_t *)(addr)) );
}

static   u32_t read32(virtual_addr_t addr)
{
	return( *((volatile u32_t *)(addr)) );
}

static   u64_t read64(virtual_addr_t addr)
{
	return( *((volatile u64_t *)(addr)) );
}



static   void write8(virtual_addr_t addr, u8_t value)
{
	*((volatile u8_t *)(addr)) = value;
}

static   void write16(virtual_addr_t addr, u16_t value)
{
	*((volatile u16_t *)(addr)) = value;
}

static   void write32(virtual_addr_t addr, u32_t value)
{
	*((volatile u32_t *)(addr)) = value;
}

static   void write64(virtual_addr_t addr, u64_t value)
{
	*((volatile u64_t *)(addr)) = value;
}

virtual_addr_t phys_to_virt(physical_addr_t phys);
physical_addr_t virt_to_phys(virtual_addr_t virt);


//���üĴ���ֵ
#define C_BIT(addr,bit)  				write32(addr,read32(addr) & (~((u64_t)(1)<<bit)) ) 
#define S_BIT(addr,bit)  				write32(addr,read32(addr) | ((u64_t)(1)<<bit)    ) 
#define C_Vlue(addr,bit,vlue)   write32(addr,read32(addr) & (~((u64_t)(vlue)<<bit)) ) 
#define S_Vlue(addr,bit,vlue)   write32(addr,read32(addr) | ((u64_t)(vlue)<<bit)    ) 



#ifdef __cplusplus
}
#endif

#endif /* __IO_H__ */
