#include "util.h"
#include "process.h"
#include "oracle_tns.h"
#include "session.h"

int mainprocess(uchar *data, uint len, struct sess_key key, enum direction dir)
{
    int ret = 0;
    char sip[16] = {0};
    char dip[16] = {0};
    struct tns_session *sess;
    //debug_hex_println("dump mainprocess data", data, len);

    memcpy(sip,inet_ntoa(*(struct in_addr*)(&key.sip)),sizeof(sip));
    memcpy(dip,inet_ntoa(*(struct in_addr*)(&key.dip)),sizeof((dip)));

    sess = sess_must_find_node(key,dir);
    sess->cur_dir = dir;
    sprintf(sess->cur_sess_desc,"[%s:%u -> %s:%u]",sip, NTOHS(key.sport), dip, NTOHS(key.dport));
    
    ret = TNS_protocol_parse(data, len, sess);
    
    return ret;
}

