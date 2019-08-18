#ifndef __SESSION_H__
#define __SESSION_H__

#include "util.h"

#define MAX_SESS_NODE 32

enum sess_link_stat
{
    STAT_CLOSE      = 0X00,
    STAT_INIT       = 0X01,
    STAT_CONNECT    = 0X02,
    STAT_RESEND     = 0X03,
    STAT_ACCEPT     = 0X04,
    STAT_REFUSE     = 0X05,
    STAT_DATA       = 0X06,
};

enum sess_login_stat
{
    stat_loging_in      = 0x10,
    stat_login_failed   = 0x20,
    stat_login_success  = 0x30,
};

struct tns_session
{
    struct list_head sess_list; //unused
    struct sess_key key;
    ushort          version;
    ushort          version_compatible;
    int             state;
    int             cur_dir;
    ulonglong       update_time;
    char            cur_sess_desc[64];
};

struct tns_session* sess_must_find_node(struct sess_key skey, int dir);

int sess_init(void);

#endif
