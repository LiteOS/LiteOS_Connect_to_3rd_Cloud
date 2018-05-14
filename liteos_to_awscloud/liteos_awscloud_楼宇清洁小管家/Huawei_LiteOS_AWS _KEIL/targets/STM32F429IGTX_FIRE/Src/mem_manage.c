#include "los_memory.h"
#include "mem_manage.h"
void liteosfree(void *parg) 
{
	(VOID)LOS_MemFree(m_aucSysMem0, (VOID *)parg);
}


void *liteoscalloc(size_t n , size_t size)
{
	return (LOS_MemAlloc (m_aucSysMem0, n* size));
}