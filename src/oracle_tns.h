#ifndef __ORACLE_TNS_H__
#define __ORACLE_TNS_H__

#include "util.h"

//Relative to tns packet head : pkt
#define TNS_HEAD_LEN            (8)
#define OFFSET_TNS_LEN          (0)
#define OFFSET_TNS_CKSUM        (2)
#define OFFSET_TNS_TYPE         (4)
#define OFFSET_TNS_RESV         (5)
#define OFFSET_TNS_HEAD_CKSUM   (6)
#define OFFSET_TNS_DATA         (TNS_HEAD_LEN)

#define OFFSET_TNS_CONNECT_VERSION              (0)
#define OFFSET_TNS_CONNECT_VERSION_COMPATIBLE   (2)
#define OFFSET_TNS_CONNECT_DATA_LEN             (16)
#define OFFSET_TNS_CONNECT_DATA_OFFSET          (18)

#define OFFSET_TNS_ACCEPT_VERSION               (0)
#define OFFSET_TNS_ACCEPT_DATA_LEN              (30)
#define OFFSET_TNS_ACCEPT_DATA_OFFSET           (32)

#define OFFSET_TNS_REFUSE_PROCESS_CAUSE         (0)
#define OFFSET_TNS_REFUSE_SYS_CAUSE             (1)
#define OFFSET_TNS_REFUSE_DATA_LEN              (2)
#define OFFSET_TNS_REFUSE_DATA_OFFSET           (4)

#define OFFSET_TNS_DATA_FALG                    (0)
#define OFFSET_TNS_DATA_ID                      (2)
#define OFFSET_TNS_DATA_ID_SUB                  (3)
#define OFFSET_TNS_DATA_OFFSET                  (4)

#define PKT_LEN(pkt)                (NTOHS(*(ushort*)(((uchar*)pkt + OFFSET_TNS_LEN))))
#define PKT_CKSUM(pkt)              (NTOHS(*(ushort*)(((uchar*)pkt + OFFSET_TNS_CKSUM))))
#define PKT_TYPE(pkt)               (*((uchar*)pkt + OFFSET_TNS_TYPE))
#define PKT_RESV(pkt)               (*((uchar*)pkt + OFFSET_TNS_RESV))
#define PKT_HEAD_CKSUM(pkt)         (NTOHS(*(ushort*)(((uchar*)pkt + OFFSET_TNS_CKSUM))))

#define PKT_DATA(pkt)               ((uchar*)pkt + OFFSET_TNS_DATA)

//Relative to tns packet data : PKT_DATA(pkt)
#define CONNECT_VER(pkt)            (NTOHS(*(ushort*)(PKT_DATA(pkt) + OFFSET_TNS_CONNECT_VERSION)))
#define CONNECT_VER_COMPATIBLE(pkt) (NTOHS(*(ushort*)(PKT_DATA(pkt) + OFFSET_TNS_CONNECT_VERSION_COMPATIBLE)))
#define CONNECT_DATA_LEN(pkt)       (NTOHS(*(ushort *)(PKT_DATA(pkt) + OFFSET_TNS_CONNECT_DATA_LEN)))
#define CONNECT_DATA_OFFSET(pkt)    (NTOHS(*(ushort*)(PKT_DATA(pkt) + OFFSET_TNS_CONNECT_DATA_OFFSET)))
#define CONNECT_DATA(pkt)           ((uchar*)pkt + CONNECT_DATA_OFFSET(pkt))

//Relative to tns packet data : PKT_DATA(pkt)
#define ACCEPT_VER(pkt)             (NTOHS(*(ushort*)(PKT_DATA(pkt) + OFFSET_TNS_CONNECT_VERSION)))
#define ACCEPT_DATA_LEN(pkt)        (NTOHS(*(ushort*)(PKT_DATA(pkt) + OFFSET_TNS_ACCEPT_DATA_LEN)))
#define ACCEPT_DATA_OFFSET(pkt)     (NTOHS(*(ushort*)(PKT_DATA(pkt) + OFFSET_TNS_ACCEPT_DATA_OFFSET)))
#define ACCEPT_DATA(pkt)            ((uchar*)pkt + ACCEPT_DATA_OFFSET(pkt))

//Relative to tns packet data : PKT_DATA(pkt)
#define REFUSE_PRO_CAUSE(pkt)       (*(PKT_DATA(pkt) + OFFSET_TNS_REFUSE_PROCESS_CAUSE))
#define REFUSE_SYS_CAUSE(pkt)       (*(PKT_DATA(pkt) + OFFSET_TNS_REFUSE_SYS_CAUSE))
#define REFUSE_DATA_LEN(pkt)        (NTOHS(*(ushort*)((uchar*)pkt + OFFSET_TNS_REFUSE_DATA_LEN)))
#define REFUSE_DATA(pkt)            ((uchar*)pkt + OFFSET_TNS_REFUSE_DATA_OFFSET)

//Relative to tns packet data : PKT_DATA(pkt)
#define DATA_FLAG(pkt)              (NTOHS(*(ushort*)(PKT_DATA(pkt) + OFFSET_TNS_DATA_FALG)))
#define DATA_ID(pkt)                (*(PKT_DATA(pkt) + OFFSET_TNS_DATA_ID))
#define DATA_ID_SUB(pkt)                (*(PKT_DATA(pkt) + OFFSET_TNS_DATA_ID_SUB))
#define DATA_DATA(pkt)              ((uchar*)pkt + OFFSET_TNS_DATA_OFFSET)

//Relative to tns packet data data : DATA_DATA(pkt)


enum Pkt_type
{
    TNS_TYPE_CONNECT    = 1,
    TNS_TYPE_ACCEPT,
    TNS_TYPE_ACK,
    TNS_TYPE_REFUSE,
    TNS_TYPE_REDIRECT,
    TNS_TYPE_DATA,
    TNS_TYPE_NULL,
    TNS_TYPE_UNKNOWN1,
    TNS_TYPE_ABORT,
    TNS_TYPE_UNKNOWN2,
    TNS_TYPE_RESEND,
    TNS_TYPE_MARKER,
    TNS_TYPE_UNKNOWN3,
    TNS_TYPE_UNKNOWN4    = 14,
};

enum Data_id_ver313
{
    TNS313_DATA_ID_SNS    = 0xde, //secure network service
    TNS313_DATA_ID_SP     = 0x01, //set protocol
    TNS313_DATA_ID_OCI    = 0x03, //OCI function
    TNS313_DATA_ID_RS     = 0x04, //return status
};
    
enum Data_id_ver314
{
    TNS314_DATA_ID_SNS    = 0xde, //secure network service
    TNS314_DATA_ID_SP     = 0x01, //set protocol
    TNS314_DATA_ID_OCI    = 0x03, //OCI function
    TNS314_DATA_ID_RS     = 0x04, //return status
};


enum Data_handle_version
{
    TNS_313 = 313,
    TNS_314 = 314
};

enum error_code
{
    CODE_SUCCESS   = 0x00,
    CODE_PKT_LEN   = 0x01,
    CODE_PKT_TYPE  = 0x02,
    CODE_DATA_ID   = 0x03,
};

struct Pkt_handle
{
    ushort  version;
    uchar   type;
    int (*func)(uchar*, uint, void*);
};

#pragma pack(push,1)
struct TNS_pkt_head
{
    ushort  len;
    ushort  pkt_cksum;
    uchar   type;
    uchar   resv;
    ushort  head_ckcum;
};
#pragma pack(pop)

int TNS_protocol_parse(uchar *data, uint len, void *userdata);
int msg_data_id_handle(struct Pkt_handle ph[], int num, ushort ver, uchar type, uchar *data, uint len, void *userdata);
int packet_handle(struct Pkt_handle ph[], int num, uchar type, uchar *data, uint len, void *userdata);

int tns_connect_handle(uchar *pkt, uint len, void *userdata);
int tns_accept_handle(uchar *pkt, uint len, void *userdata);
int tns_refuse_handle(uchar *pkt, uint len, void *userdata);
int tns_data_handle(uchar *pkt, uint len, void *userdata);
int tns_resend_handle(uchar *pkt, uint len, void *userdata);

int tns_313_RS(uchar *pkt, uint len, void *userdata);
int tns_313_OCI(uchar *pkt, uint len, void *userdata);
int tns_313_SP(uchar *pkt, uint len, void *userdata);
int tns_313_SNS(uchar *pkt, uint len, void *userdata);

int tns_314_RS(uchar *pkt, uint len, void *userdata);
int tns_314_OCI(uchar *pkt, uint len, void *userdata);
int tns_314_SP(uchar *pkt, uint len, void *userdata);
int tns_314_SNS(uchar *pkt, uint len, void *userdata);

#endif
