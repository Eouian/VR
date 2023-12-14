#include "sys_cache.h"
#include "arm32.h"


extern void sysSetupCP15(unsigned int);
extern void sys_flush_and_clean_dcache(void);

void sysFlushCache(INT32 nCacheType)
{
	int temp;

	switch (nCacheType)
	{
		case I_CACHE:
			__asm
			{
				/*----- flush I-cache -----*/
				MOV temp, 0x0
				MCR p15, 0, temp, c7, c5, 0 /* invalidate I cache */
			}
			break;

		case D_CACHE:
			sys_flush_and_clean_dcache();
			__asm
			{
				/*----- flush D-cache & write buffer -----*/
				MOV temp, 0x0			
				MCR p15, 0, temp, c7, c10, 4 /* drain write buffer */
			}
			break;

		case I_D_CACHE:
			sys_flush_and_clean_dcache();
			__asm
			{

				/*----- flush I, D cache & write buffer -----*/
				MOV temp, 0x0
				MCR p15, 0, temp, c7, c5, 0 /* invalidate I cache */
				MCR p15, 0, temp, c7, c10, 4 /* drain write buffer */		
			}
			break;

		default:
			;
	}
}

void sysInvalidCache(void)
{
	int temp;

	__asm
	{		
		MOV temp, 0x0
		MCR p15, 0, temp, c7, c7, 0 /* invalidate I and D cache */
	}
}
