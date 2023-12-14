#ifndef __SYS_CACHE_H__
#define __SYS_CACHE_H__

#include "sys_types.h"


#define I_CACHE			6
#define D_CACHE			7
#define I_D_CACHE		8



void sysFlushCache(INT32 nCacheType);
void  sysInvalidCache(void);

#endif
