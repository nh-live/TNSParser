#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <pthread.h>
#include "common.h"

#define RING_QUEUE_SIZE 32
#define RING_BUF_SIZE 1500

typedef struct MyList {
    struct Packet *pkt;
    struct MyList *next;
} MyList;

typedef struct ListQueue {
    MyList *first, *last;
    int num;
    int abort_request;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} ListQueue;

typedef struct RingQueue {
    struct Packet queue[RING_QUEUE_SIZE];
    int rindex;
    int windex;
    int size;
    int max_size;
    int abort_request;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} RingQueue;

int list_queue_init(ListQueue *q);
void list_queue_destroy(ListQueue *q);
void list_queue_abort(ListQueue *q);
void list_queue_flush(ListQueue *q);
int list_queue_put(ListQueue *q, struct Packet **pkt);
int list_queue_get(ListQueue *q, struct Packet **pkt, int block);

int ring_queue_init(RingQueue *q, int max_size);
void ring_queue_destroy(RingQueue *q);
void ring_queue_signal(RingQueue *q);
struct Packet *ring_queue_peek_writable(RingQueue *q);
struct Packet *ring_queue_peek_readable(RingQueue *q);
void ring_queue_push(RingQueue *q);
void ring_queue_pull(RingQueue *q);

#endif



