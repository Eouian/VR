#include <sys_mmu.h>
#include "arm32.h"
#include "sys_io.h"
#include "sys_cache.h"

#define F1C100S_DRAM_32MB 32 //f1c100s dram size=32MB
#define F1C200S_DRAM_64MB 64 //f1c200s dram size=64MB

#define DRAM_ADDR (0x80000000) //DRAM起始地址
#define DRAM_SIZE (F1C100S_DRAM_32MB) //定义DRAM大小
#define MMU_TAB_SIZE (0x4000) //映射表大小 16KB

//测试只用一级映射更快

//Cache写机制
#define F1C100S_CACHE_WRITE_BACK 3     //写回模式
#define F1C100S_CACHE_WRITE_THROUGH 2  //写通模式
//		
#define SZ_1M (unsigned int)(1024*1024)
#define SZ_2G (unsigned int)(2048*SZ_1M)


#ifdef FIXED_MMU_TTB //BOOT0需要开启MMU时，SRAM区存放不下TTB映射表，需要指定到DRAM区。
    unsigned int* mmuTable=(unsigned int * )(DRAM_ADDR+(DRAM_SIZE*SZ_1M)-MMU_TAB_SIZE);//DRAM最后16KB用于存放映射表TTB
#else
    __align(0x4000) unsigned int mmuTable[4096];
#endif

/*
页表初始化
virt 虚拟地址
phys 物理地址
size 大小
type 写机制类型
*/
static void map_l1_section(uint32_t * ttb, virtual_addr_t virt, physical_addr_t phys, physical_size_t size, unsigned char type)
{
	physical_size_t i;

	virt >>= 20;
	phys >>= 20;
	size >>= 20;
	type &= 0x3;

	for(i = size; i > 0; i--, virt++, phys++)
		ttb[virt] = (phys << 20) | (0x3 << 10) | (0x0 << 5) | (0x1 << 4) | (type << 2) | (0x2 << 0) ;//位5-全部域0
}

/*
mmu初始化
*/
void sys_mmu_init(void)
{
	sysFlushCache(D_CACHE);
	arm32_dcache_disable();	
	arm32_icache_disable();
	arm32_mmu_disable();
	
	unsigned int * ttb=mmuTable;
	
	map_l1_section(ttb, 0x00000000, 0x00000000, SZ_2G, 0);
	map_l1_section(ttb, 0x80000000, 0x80000000, SZ_2G, 0);
	
#if 0	
	map_l1_section(ttb, 0x00000000, 0x00000000, SZ_1M * 1        , F1C100S_CACHE_WRITE_BACK);
	map_l1_section(ttb, 0x80000000, 0x80000000, SZ_1M * DRAM_SIZE, F1C100S_CACHE_WRITE_BACK);
#else
	map_l1_section(ttb, 0x00000000, 0x00000000, SZ_1M * 1        , F1C100S_CACHE_WRITE_THROUGH);
	map_l1_section(ttb, 0x80000000, 0x80000000, SZ_1M * DRAM_SIZE, F1C100S_CACHE_WRITE_THROUGH); 
#endif
	
	arm32_ttb_set((uint32_t)(ttb));//写入页表地址
	arm32_tlb_invalidate();
	arm32_domain_set(0x1);//设置域不进行权限检查
	arm32_mmu_enable();
	arm32_icache_enable();
	arm32_dcache_enable();
}
