#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <netpacket/packet.h>
#include <fcntl.h>
#include <errno.h>

#include "socket.h"

const char  NETIF_NAME[16]        = "docker0";
static int g_netif_index = 0;

static int active_dev(int sockFd, const char *dev, int *index)
{
    int     ret;
    struct  ifreq req;

    bzero(&req, sizeof(struct ifreq));

    strncpy(req.ifr_name, dev, sizeof(req.ifr_name));

    ret = ioctl(sockFd, SIOCGIFFLAGS, &req);
    if (ret == -1)
    {
        return(-1);
    }
    req.ifr_flags |= IFF_UP;
    req.ifr_flags |= IFF_PROMISC;

    ret = ioctl(sockFd, SIOCSIFFLAGS, &req);
    if (ret == -1)
    {
        //alarm_log(PERR_MSG, MODULE_SOCKET, "ioctl:SIOCSIFFLAGS");
        return(-1);
    }

    ioctl(sockFd, SIOCGIFINDEX, &req);
    printf("****   if name %s, if index %d   *****\n", req.ifr_name, req.ifr_ifindex);
    
    *index = req.ifr_ifindex;    
    

    return(0);
}

int send_via_dev(const int sockFd, uchar* data, int len)
{
    return 0;
}

int recv_via_dev(const int sockFd, uchar* data, int len)
{
    int ret = 0;
    struct sockaddr_ll sa;
    socklen_t sz = sizeof(sa);

    ret = recvfrom(sockFd, data, len, 0, (struct sockaddr*)&sa, &sz);   //function defined in system
    if (ret == -1)
    {
        return -1;
    }

    return(ret);
}

int creat_sock(void)
{
    int fd;
    fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (fd < 0)
    {
        perror("ethernet:cant get SOCK_PACKET socket");
    }
    else 
    {
        active_dev(fd, NETIF_NAME, &g_netif_index);
    }   

    printf("**** NETIF_NAME %s, g_netif_index %d   *****\n", NETIF_NAME, g_netif_index);


    return fd;
}

void set_sock_nonblock(int sockFd)
{
    
    int flags = fcntl(sockFd, F_GETFL, 0);
    printf("ethernet get flag is %08x\n", flags);
    fcntl(sockFd, F_SETFL, flags|O_NONBLOCK);
    printf("ethernet set flag is %08x\n", flags);
    unsigned long has = 1;
    ioctl(sockFd, FIONBIO , &has);

    return;
}

