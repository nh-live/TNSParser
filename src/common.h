#ifndef __COMMON_H__
#define __COMMON_H__

#include <netinet/in.h>

typedef signed char         s8;
typedef unsigned char       u8;
typedef signed short        s16;
typedef unsigned short      u16;
typedef signed int          s32;
typedef unsigned int        u32;
typedef signed long long    s64;
typedef unsigned long long  u64;

typedef signed char         schar;
typedef unsigned char       uchar;
typedef signed short        sshort;
typedef unsigned short      ushort;
typedef signed int          sint;
typedef unsigned int        uint;
typedef signed long long    slonglong;
typedef unsigned long long  ulonglong;

typedef enum {
    FALSE   = 0,
    TRUE    = !FALSE
}BOOL;

enum {
    RC_ERR  = -1,
    RC_OK   = 0,
    RC_CODE = 8,
};

#define MAX(a,b)    ((a) > (b) ? (a) : (b))
#define MAX3(a,b,c) MAX(MAX(a,b),c)
#define MIN(a,b)    ((a) > (b) ? (b) : (a))
#define MIN3(a,b,c) MIN(MIN(a,b),c)

#define NTOHS(val)  ntohs(val)
#define HTONS(val)  htons(val)
#define NTOHL(val)  ntohl(val)
#define HTONL(val)  htonl(val)

#define MAX_PKTPOOL_SIZE    32
#define MAX_PACKET_SIZE     1500

enum direction {
    REQUEST     = 0,
    RESPONSE    = 1,
};
    
struct tcp_hdr {
    uint16_t src_port;  /**< TCP source port. */
    uint16_t dst_port;  /**< TCP destination port. */
    uint32_t sent_seq;  /**< TX data sequence number. */
    uint32_t recv_ack;  /**< RX data acknowledgement sequence number. */
    uint8_t  data_off;  /**< Data offset. */
    uint8_t  tcp_flags; /**< TCP flags */
    uint16_t rx_win;    /**< RX flow control window. */
    uint16_t cksum;     /**< TCP checksum. */
    uint16_t tcp_urp;   /**< TCP urgent pointer, if any. */
} __attribute__((__packed__));

struct sess_key {
    uint32_t sip;
    uint32_t dip;
    uint16_t sport;
    uint16_t dport;
};

typedef struct Packet {
    uchar   buf[MAX_PACKET_SIZE];
    uchar   *data;                  //pkt head
    uint    len;                    //pkt len
    int     dir;                    //pkt direction
    int     action;
    struct sess_key key;
} Packet;

#endif
