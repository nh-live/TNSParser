#include "common.h"
#include "util.h"
#include "network.h"
#include "process.h"
#include "session.h"

Networks *network;
ListQueue lqr;

void *g_pktpool = NULL;

void *process_thread(void *arg)
{
    struct Packet *pkt = NULL;

    list_queue_get(&lqr, &pkt, 1);
    while(1)
    {
        if (pkt->len != 0 && pkt->action != RESULT_DROP)
        {
            mainprocess((unsigned char *)pkt->data, pkt->len, pkt->key, pkt->dir);
        }
        mem_pool_put(g_pktpool, pkt);
        list_queue_get(&lqr, &pkt, 1);
    }
    return NULL;
}

void *netrecv_thread(void *arg)
{
    int ret = 0;
    Networks *net = NULL;
    struct Packet *pkt = NULL;

    net = network;
    pkt = mem_pool_get(g_pktpool);
    while(1)
    {
        ret = net->recv(net->handler, pkt);
        if (ret)
        {
            usleep(10);
            continue;
        }
        list_queue_put(&lqr, &pkt);
        pkt = mem_pool_get(g_pktpool);
    }
    mem_pool_put(g_pktpool, pkt);
    return NULL;
}

int main(int argc, char **argv)
{
    pthread_t netr_threadid;
    pthread_t process_threadid;
    
    if (argc < 3)
    {
        printf("usage:\ntnsparser target_ip target_port\n");
        return 0;
    }

    //init
    g_target_ip = (inet_addr(argv[1]));
    g_target_port = atoi(argv[2]);
    printf("target_ip :%s\n",argv[1]);
    printf("target_ip :0x%08x\n",g_target_ip);
    printf("target_port :%u\n",g_target_port);
    
    network = get_network();
    g_pktpool = mem_pool_create(MAX_PKTPOOL_SIZE, sizeof(struct Packet));

    list_queue_init(&lqr);
    sess_init();

    network->init();
    network->open(&network->handler);
    
    lqr.abort_request = 0;

    pthread_create(&netr_threadid, NULL, netrecv_thread, NULL);
    pthread_create(&process_threadid, NULL, process_thread, NULL);

    while(1)
    {
        sleep(10);
    }
    
    list_queue_abort(&lqr);

    network->close(network->handler);
    network->deinit();

    list_queue_destroy(&lqr);
    mem_pool_destroy(g_pktpool);

    return 0;
}
