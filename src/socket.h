#ifndef __SOCKET_H_
#define __SOCKET_H_

#include "util.h"

int send_via_dev(const int sockFd, uchar* data, int len);
int recv_via_dev(const int sockFd, uchar* data, int len);
void set_sock_nonblock(int sockFd);
int creat_sock(void);

#endif

