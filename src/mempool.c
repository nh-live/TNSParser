#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "list.h"
#include "mempool.h"

struct mem_pool_st
{
    struct list_head free;
    struct list_head use;
};

pthread_mutex_t g_poollock;
pthread_cond_t g_poolcond;

void* mem_pool_create(int num, int size)
{
    int i;
    void* mem;
    struct mem_pool_st* mem_pool;
    struct list_head* list;

    if (num <= 0)
    {
        return NULL;
    }
    pthread_mutex_init(&g_poollock, NULL);
    pthread_cond_init(&g_poolcond, NULL);

    size = ((size + 15) & 0xFFFFFFF0) + sizeof(struct list_head);
    mem = malloc(num * size + sizeof(struct mem_pool_st));
    mem_pool = (struct mem_pool_st*)mem;
    list = (struct list_head*)(mem_pool + 1);
    INIT_LIST_HEAD(&mem_pool->free);
    INIT_LIST_HEAD(&mem_pool->use);
    for (i = 0; i < num; i ++)
    {
        list_add(list, &mem_pool->free);
        list = (struct list_head*)((char*)list + size);
    }

    return mem;
}

void* mem_pool_get(void* cache)
{
    struct list_head* list;
    struct mem_pool_st* mem_pool;

    mem_pool = (struct mem_pool_st*)cache;

    pthread_mutex_lock(&g_poollock);
    if (list_empty(&mem_pool->free))
    {
        pthread_cond_wait(&g_poolcond, &g_poollock);
//        return NULL;
    }

    list = mem_pool->free.next;
    list_del(list);
    list_add(list, &mem_pool->use);
    pthread_mutex_unlock(&g_poollock);

    return (void*)(list + 1);
}

int mem_pool_put(void* cache, void* node)
{
    struct list_head* list;
    struct mem_pool_st* mem_pool;

    if (cache == NULL || node == NULL)
    {
        return 1;
    }

    mem_pool = (struct mem_pool_st*)cache;
    list = (struct list_head*)((char*)node - sizeof(struct list_head));
    if (NULL == list->prev && NULL == list->next)
    {
        free(list);
        return 0;
    }
    pthread_mutex_lock(&g_poollock);
    list_del(list);
    list_add(list, &mem_pool->free);
    pthread_cond_signal(&g_poolcond);
    pthread_mutex_unlock(&g_poollock);
    return 0;
}

void* mem_pool_clone(void *node, int size)
{
    void *mem;
    struct list_head* list;

    mem = malloc(size + sizeof(struct list_head));
    if (NULL == mem)
    {
        return NULL;
    }
    list = (struct list_head *)mem;
    list->prev = NULL;
    list->next = NULL;
    memcpy((void*)(list + 1), node, size);

    return (void*)(list + 1);
}

void mem_pool_destroy(void* mem)
{
    pthread_mutex_destroy(&g_poollock);
    pthread_cond_destroy(&g_poolcond);
    return free(mem);
}


