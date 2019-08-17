#include "util.h"

typedef enum ACTION {
    DROP        = 1 << 0,
    TRANSMIT    = 1 << 1,
    RESPOSE     = 1 << 2,
}action;
enum
{
    RESULT_PUSH,
    RESULT_TRANSIT,
    RESULT_DROP,
    RESULT_WAIT
};

typedef struct Networks {
    const char *name;   //安全模块处理名字
    void *handler;

/*  function    : init
*   param:
*/
    int (*init)();
/*  function    : deinit
*   param:
*/
    int (*deinit)();
/*  function    : open
*   param:
*       handler : 网络句柄
*/
    int (*open)(void **handler);
/*  function    : clsoe
*   param:
*       handler : 网络句柄
*/
    int (*close)(void *handler);
/*  function    : recv
*   param:
*       handler : 网络句柄
*       pkt     : 接收数据包
*   return:
*       失败返回负数
*       成功返回实际接收数据长度
*/
    int (*recv)(void *handler, struct Packet *pkt);
/*  function    : send
*   param:
*       handler : 网络句柄
*       pkt     : 发送数据包
*   return:
*       失败返回负数
*       成功返回实际发送数据长度
*/
    int (*send)(void *handler, struct Packet *pkt);
}Networks;

typedef struct eth_handle
{
    int fd;
}eth_handle;

Networks *get_network();

