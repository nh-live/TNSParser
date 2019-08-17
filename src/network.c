#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netpacket/packet.h>

#include "network.h"
#include "socket.h"
#include "session.h"

//pack len
#define TRY_RECV_LEN				  1518 
#define ETH_MAC_LEN                   12 
#define ETH_HEAD_LEN				  14
#define IP_HEAD_LEN				      20
#define TCP_HEAD_LEN                  20
#define IP_TCP_HEAD_LEN               (IP_HEAD_LEN+TCP_HEAD_LEN)
#define ETH_IP_HEAD_LEN               (ETH_HEAD_LEN+IP_HEAD_LEN)
#define ETH_IP_TCP_HEAD_LEN           (ETH_HEAD_LEN+IP_HEAD_LEN+TCP_HEAD_LEN)
#define IPV4 4

#define IP_OFFSET(pack)    ((struct iphdr *)((uchar*)pack+ETH_HEAD_LEN))
#define TCP_OFFSET(ip)     ((struct tcphdr *)((uchar *)ip + (ip->ihl << 2)))
#define TCP_DATA(pack)     (((uchar*)pack + ETH_IP_HEAD_LEN + ((struct tcphdr *)((uchar*)pack + ETH_IP_HEAD_LEN))->doff*4))

static void recv_set_pack_info(struct Packet *pkt, int dataLen, int action)
{
    struct iphdr   *ip = IP_OFFSET(pkt->buf);
    struct tcphdr  *tcp = TCP_OFFSET(ip);
    
    pkt->len  = dataLen;
    pkt->data = pkt->buf+ETH_HEAD_LEN + (ip->ihl << 2) + tcp->doff*4;
    pkt->action = action;

}

static int ip_recv(uchar *pack, int len, int *dataLen)
{
    struct iphdr   *ip = IP_OFFSET(pack);
    struct tcphdr  *tcp;
    struct tcp_hdr *htcp;

    //sess_node *sess;
    int ret = RESULT_DROP;
    unsigned short tcp_len;

    if(IPV4 != ip->version)
        return RESULT_DROP;

    
    if (IPPROTO_TCP != ip->protocol)
    {   
        return RESULT_DROP;
    }

    tcp = TCP_OFFSET(ip);
    htcp = (struct tcp_hdr *)tcp;

    tcp_len = ntohs(ip->tot_len)-(ip->ihl << 2)-tcp->doff*4;

    *dataLen = tcp_len;

    if (NTOHS(htcp->src_port) == 1521 ||
        NTOHS(htcp->dst_port) == 1521)
        return RESULT_PUSH;
    else
		;//printf("tcp->port %u -> %u\n",NTOHS(htcp->src_port), NTOHS(htcp->dst_port));
    
    return ret;
}

static int proc_incoming_pack(int fd, uchar *pack, int len, int *dataLen)
{
    struct ethhdr *     eth;
    int      eproto;
    int ret = RESULT_DROP;

    eth =(struct ethhdr*)pack;
    eproto=ntohs(eth->h_proto);
    
    switch( eproto)
    {
        case ETH_P_IP:
            ret = ip_recv(pack, len, dataLen);    
            break;
        case ETH_P_ARP:
            ret = RESULT_DROP;
            break;
        case ETH_P_RARP:
            ret = RESULT_DROP;
            break;
    }
    return ret;
}

int eth_init()
{
    return 0;
}

int eth_deinit()
{
    return 0;
}

int eth_open(void **handler)
{
    eth_handle *handle = (eth_handle *)malloc(sizeof(eth_handle));
    handle->fd = creat_sock();
    if (handle->fd < 0)
    {
        perror("ethernet:cant get SOCK_PACKET socket");
        exit(0);
    }

    //set_sock_nonblock(handle->fd);
    *handler = handle;

    return 0;
}

int eth_close(void *handler)
{
    free(handler);
    return 0;
}

int eth_recv(void *handler, struct Packet *pkt)
{
    int recvLen;
    int dataLen = 0;
    eth_handle *rcv_fd = (eth_handle *)handler;
    int ret;
    int action = 0;
    
    recvLen = recv_via_dev(rcv_fd->fd, pkt->buf, TRY_RECV_LEN);
    if (recvLen < 0)
    {                    
        return 1;
    }
    action = proc_incoming_pack(rcv_fd->fd, pkt->buf, recvLen, &dataLen);
    ret = 0;
    recv_set_pack_info(pkt, dataLen, action);
    return ret;
}

int eth_send(void *handler, struct Packet *pkt)
{
    return 0;
}

Networks ethernet = {
    .name = "ethernet",
    .init = eth_init,
    .deinit = eth_deinit,
    .open = eth_open,
    .close = eth_close,
    .recv = eth_recv,
    .send = eth_send,
};

Networks *get_network()
{
    return &ethernet;
}
