#ifndef __MEMPOOL_H__
#define __MEMPOOL_H__

void* mem_pool_create(int num, int size);
void* mem_pool_get(void* cache);
int   mem_pool_put(void* cache, void* node);
void* mem_pool_clone(void *node, int size);
void  mem_pool_destroy(void* mem);

#endif
