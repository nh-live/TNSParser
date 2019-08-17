#ifndef __SESSION_H__
#define __SESSION_H__

#include "util.h"

enum sess_link_stat
{
    stat_init       = 0x00,
    stat_connect    = 0x01,
    stat_resend     = 0x02,
    stat_accept     = 0x02,
    stat_refuse     = 0x03,
    stat_data       = 0x04,
};

enum sess_login_stat
{
    stat_loging_in      = 0x00,
    stat_login_failed   = 0x01,
    stat_login_success  = 0x02,
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

struct tns_session
{
    struct sess_key key;
    uchar           tns_version;
    uchar           tns_version_compatible;
    uint            tcp_state;              //to do
    uint            link_state;
    uint            login_state;
    ulonglong       update_time;
};

#endif
