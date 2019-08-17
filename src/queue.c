#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "queue.h"

static int list_queue_put_private(ListQueue *q, struct Packet **pkt)
{
    MyList *ml;

    if (q->abort_request)
       return -1;

    ml = (MyList *)malloc(sizeof(MyList));
    if (!ml)
        return -1;
    ml->pkt = *pkt;
    ml->next = NULL;

    if (!q->last)
        q->first = ml;
    else
        q->last->next = ml;
    q->last = ml;
    q->num++;
    /* XXX: should duplicate packet data in DV case */
    pthread_cond_signal(&q->cond);
    return 0;
}

int list_queue_put(ListQueue *q, struct Packet **pkt)
{
    int ret;

    pthread_mutex_lock(&q->mutex);
    ret = list_queue_put_private(q, pkt);
    pthread_mutex_unlock(&q->mutex);
    return ret;
}

/* packet queue handling */
int list_queue_init(ListQueue *q)
{
    memset(q, 0, sizeof(ListQueue));
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
    q->abort_request = 1;
    return 0;
}

void list_queue_flush(ListQueue *q)
{
    MyList *pkt, *pkt1;

    pthread_mutex_lock(&q->mutex);
    for (pkt = q->first; pkt; pkt = pkt1) {
        pkt1 = pkt->next;
        free(pkt);
    }
    q->last = NULL;
    q->first = NULL;
    q->num = 0;
    pthread_mutex_unlock(&q->mutex);
}

void list_queue_destroy(ListQueue *q)
{
    list_queue_flush(q);
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->cond);
}

void list_queue_abort(ListQueue *q)
{
    pthread_mutex_lock(&q->mutex);
    q->abort_request = 1;
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->mutex);
}

/* return < 0 if aborted, 0 if no packet and > 0 if packet.  */
int list_queue_get(ListQueue *q, struct Packet **pkt, int block)
{
    MyList *pkt1;
    int ret;

    pthread_mutex_lock(&q->mutex);
    for (;;) {
        if (q->abort_request) {
            ret = -1;
            break;
        }

        pkt1 = q->first;
        if (pkt1) {
            q->first = pkt1->next;
            if (!q->first)
                q->last = NULL;
            q->num--;
            *pkt = pkt1->pkt;
            free(pkt1);
            ret = 1;
            break;
        } else if (!block) {
            ret = 0;
            break;
        } else {
            pthread_cond_wait(&q->cond, &q->mutex);
        }
    }
    pthread_mutex_unlock(&q->mutex);
    return ret;
}

int ring_queue_init(RingQueue *q, int max_size)
{
//    int i;
    memset(q, 0, sizeof(RingQueue));
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
    q->abort_request = 1;
    q->max_size = MIN(max_size, RING_QUEUE_SIZE);
/*    for (i = 0; i < q->max_size; i++)
    {
        q->queue[i].len = RING_BUF_SIZE;
        if (!(q->queue[i].buf = (char *)malloc(RING_BUF_SIZE)))
        {
            return ENOMEM;
        }
    }*/
    return 0;
}

void ring_queue_destroy(RingQueue *q)
{
/*    int i;
    for (i = 0; i < q->max_size; i++) {
        free(q->queue[i].buf);
        q->queue[i].buf = NULL;
        q->queue[i].len = 0;
    }*/
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->cond);
}

void ring_queue_signal(RingQueue *q)
{
    pthread_mutex_lock(&q->mutex);
    q->abort_request = 1;
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->mutex);
}

struct Packet *ring_queue_peek_writable(RingQueue *q)
{
    /* wait until we have space to put a new frame */
    pthread_mutex_lock(&q->mutex);
    while (q->size >= q->max_size
            && !q->abort_request) {
        pthread_cond_wait(&q->cond, &q->mutex);
    }
    pthread_mutex_unlock(&q->mutex);

    if (q->abort_request)
        return NULL;

    return &q->queue[q->windex];
}

struct Packet *ring_queue_peek_readable(RingQueue *q)
{
    /* wait until we have a readable a new frame */
    pthread_mutex_lock(&q->mutex);
    while (q->size <= 0 
            && !q->abort_request) {
        pthread_cond_wait(&q->cond, &q->mutex);
    }
    pthread_mutex_unlock(&q->mutex);

    if (q->abort_request)
        return NULL;

    return &q->queue[q->rindex];
}

void ring_queue_push(RingQueue *q)
{
    if (++q->windex == q->max_size)
        q->windex = 0;
    pthread_mutex_lock(&q->mutex);
    q->size++;
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->mutex);
}

void ring_queue_pull(RingQueue *q)
{
    if (++q->rindex == q->max_size)
        q->rindex = 0;
    pthread_mutex_lock(&q->mutex);
    q->size--;
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->mutex);
}


