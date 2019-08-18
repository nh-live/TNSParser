#include "oracle_tns.h"

struct Pkt_handle msg_type_handles[] =
{
    {0, TNS_TYPE_CONNECT,  tns_connect_handle},
    {0, TNS_TYPE_ACCEPT,   tns_accept_handle},
    {0, TNS_TYPE_ACK,      NULL},
    {0, TNS_TYPE_REFUSE,   tns_refuse_handle},
    {0, TNS_TYPE_REDIRECT, NULL},
    {0, TNS_TYPE_DATA,     tns_data_handle},
    {0, TNS_TYPE_NULL,     NULL},
    {0, TNS_TYPE_UNKNOWN1,  NULL},
    {0, TNS_TYPE_ABORT,    NULL},
    {0, TNS_TYPE_UNKNOWN2,  NULL},
    {0, TNS_TYPE_RESEND,   tns_resend_handle},
    {0, TNS_TYPE_MARKER,   NULL},
    {0, TNS_TYPE_UNKNOWN3,  NULL},
    {0, TNS_TYPE_UNKNOWN4,  NULL}
};

struct Pkt_handle msg_data_id_handles[] = 
{
    {TNS_313, TNS313_DATA_ID_SNS,   tns_313_SNS},   //secure network service
    {TNS_313, TNS313_DATA_ID_SP,    tns_313_SP},    //set protocol
    {TNS_313, TNS313_DATA_ID_OCI,   tns_313_OCI},   //OCI function
    {TNS_313, TNS313_DATA_ID_RS,    tns_313_RS},    //return status

    {TNS_314, TNS314_DATA_ID_SNS,   tns_314_SNS},   //secure network service
    {TNS_314, TNS314_DATA_ID_SP,    tns_314_SP},    //set protocol
    {TNS_314, TNS314_DATA_ID_OCI,   tns_314_OCI},   //OCI function
    {TNS_314, TNS314_DATA_ID_RS,    tns_314_RS}     //return status
};


char *direction_str[] = 
{
    "REQUEST",
    "RESPONSE"
};

static int parse_connect_data_to_json(char *bufin, int inlen, char *bufout, int *outlen)
{
    int i = 0;
    char *p, *q = NULL;
    
    if (bufin == NULL && bufout != NULL && inlen != 0 && *outlen != 0)
        return -1;
    
    p = bufin;
    q = bufout;
    memset(bufout,0,*outlen);

    if (*p != '(')
        return -2;
    strcat(q,"{\"");
    p++;
    for (; i  < inlen -2; i++ )
    {
        if ((q - bufout) + 2 < *outlen)
        switch(*(p + i))
        {
            case '(':
                //if (*(p+i-1) == '=')
                //    strcat(q,"{\"");
                //else 
                if (*(p+i-1) == ')')
                    ;//strcat(q,"\",\"");
                else
                    strcat(q,"{\"");
                break;
            case '=':
                if (*(p+i+1) == '(')
                    strcat(q,"\":");
                else
                    strcat(q,"\":\"");
                break;
            case ')':
                if (*(p+i+1) == '(')
                {
                    if (*(p+i-1) == ')')
                        strcat(q,",\"");
                    else
                        strcat(q,"\",\"");
                }
                else if (*(p+i-1) == ')')
                {
                    strcat(q,"}");
                }
                else
                    strcat(q,"\"}");
                break;
            default:
                strncat(q,p+i,1);
                break;
        }
    }
    if (*(p+i) != ')')
        return -2;
    if (*(p+i-1) == ')')
        strcat(q,"}");
    else
        strcat(q,"\"}");
    *outlen = strlen(bufout)+1;
    return 0;
}

static int parse_continuous_visible_characters(char *bufin, int inlen, char **ptr, int *outlen)
{
    int tmp = 0;
    int continuous_times = 0;
    char *p = NULL;

    p = bufin;
    *ptr = p;
    for (p = bufin; (p - bufin) < inlen; p++)
    {
        if (*p >= 0x20 && *p <= 0x7e)
        {
            tmp++;
        }
        else
        {
            if (tmp >= continuous_times)
            {
                continuous_times = tmp;
                *ptr = p - continuous_times;
            }
            tmp = 0;
        }
    }
    if (tmp != 0)
    {
        if (tmp >= continuous_times)
        {
            continuous_times = tmp;
            *ptr = p - continuous_times;
        }
    }
    *outlen = continuous_times + 1;
    return 0;
}

int tns_313_RS(uchar *pkt, uint len, void *userdata)
{
    int err_code = CODE_SUCCESS;
    int result_len = 0;
    char *ptr_sqlresult = NULL;
    char sql_res[128] = {0};
    struct tns_session *sess = (struct tns_session*)userdata;

    if (len <= 0)
        goto err;

    parse_continuous_visible_characters((char*)pkt, len, &ptr_sqlresult, &result_len);
    
    result_len  = result_len < (sizeof(sql_res)-1) ? result_len : (sizeof(sql_res)-1);
    snprintf(sql_res, result_len, "%s", ptr_sqlresult);

    printf("%s %s tns data : version :%d",
            sess->cur_sess_desc,
            direction_str[sess->cur_dir],
            sess->version);
    printf("    return status len:%d %s\n",result_len,sql_res);
err:
    return err_code;
}

int tns_313_OCI(uchar *pkt, uint len, void *userdata)
{
    int err_code = CODE_SUCCESS;
    int sql_len = 0;
    char *ptr_sqlstart = NULL;
    char sql_str[1024] = {0};
    struct tns_session *sess = (struct tns_session*)userdata;

    if (len <= 0)
        goto err;
    
    //todo extra sql
    if (*(pkt+3) == 0x5e)
    {
        parse_continuous_visible_characters((char*)pkt, len, &ptr_sqlstart, &sql_len);
        
        sql_len  = sql_len < (sizeof(sql_str)-1) ? sql_len : (sizeof(sql_str)-1);
        snprintf(sql_str, sql_len, "%s", ptr_sqlstart);
        
        printf("%s %s tns data : version :%d",
                sess->cur_sess_desc,
                direction_str[sess->cur_dir],
                sess->version);
        printf("    sql len:%d %s\n",sql_len,sql_str);
    }
    else if (*(pkt+3) == 0x73)
    {
        printf("%s %s tns data : version :%d",
                sess->cur_sess_desc,
                direction_str[sess->cur_dir],
                sess->version);
        sess->state = stat_loging_in;
        printf("    loging on\n");
    }
err:
    return err_code;
}
int tns_313_SP(uchar *pkt, uint len, void *userdata)
{
    int err_code = CODE_SUCCESS;
    struct tns_session *sess = (struct tns_session*)userdata;

    if (len <= 0)
        goto err;

    printf("%s %s tns data : version :%d",
            sess->cur_sess_desc,
            direction_str[sess->cur_dir],
            sess->version);
    if (sess->cur_dir == REQUEST)
    {
        printf("    Client Platform %s\n",pkt+3+(strlen((char*)pkt+3)+1));
    }
    else
    {
        printf("    Server Banner %s\n",pkt+3+(strlen((char*)pkt+3)+1));
    }

err:
    return err_code;
}
int tns_313_SNS(uchar *pkt, uint len, void *userdata)
{
    int err_code = CODE_SUCCESS;
    uchar   client_ver[4] = {0};
    uchar   server_ver[4] = {0};
    struct tns_session *sess = (struct tns_session*)userdata;

    if (len <= 0)
        goto err;

    printf("%s %s tns data : version :%d",
            sess->cur_sess_desc,
            direction_str[sess->cur_dir],
            sess->version);
    if (sess->cur_dir == REQUEST)
    {
        memcpy(client_ver, pkt+8, sizeof(client_ver));
        printf("    Client Version: %d.%d.%d.%d\n",client_ver[0],client_ver[1],client_ver[2],client_ver[3]);
    }
    else
    {
        memcpy(server_ver, pkt+8, sizeof(server_ver));
        printf("    Server Version: %d.%d.%d.%d\n",server_ver[0],server_ver[1],server_ver[2],server_ver[3]);
    }
    
err:
    return err_code;
}

int tns_314_RS(uchar *pkt, uint len, void *userdata)
{
    int err_code = CODE_SUCCESS;
    char *ptr_sqlresult = NULL;
    int result_len = 0;
    
    if (len <= 0)
        goto err;
    if (*(pkt+3) == 0x5e || *(pkt+3) == 0x04)
    {
        parse_continuous_visible_characters((char*)pkt, len, &ptr_sqlresult, &result_len);
        printf("return status : %d\n%s\n",result_len,ptr_sqlresult);
    }
err:
    return err_code;
}

int tns_314_OCI(uchar *pkt, uint len, void *userdata)
{
    int err_code = CODE_SUCCESS;
    int sql_len = 0;
    char *ptr_sqlstart = NULL;
    
    if (len <= 0)
        goto err;
    
    //extra sql
    if (*(pkt+3) == 0x5e)
    {
        parse_continuous_visible_characters((char*)pkt, len, &ptr_sqlstart, &sql_len);
        printf("query sql :len %d %s\n",sql_len,ptr_sqlstart);
    }
err:
    return err_code;
}
int tns_314_SP(uchar *pkt, uint len, void *userdata)
{
    int err_code = CODE_SUCCESS;
    struct tns_session *sess = (struct tns_session*)userdata;

    if (len <= 0)
        goto err;
    
    if (sess->cur_dir == REQUEST)
    {
        printf("client platform %s\n",pkt+3+(strlen((char*)pkt+3)+1));
    }
    else
    {
        printf("server platform %s\n",pkt+3+(strlen((char*)pkt+3)+1));
    }

err:
    return err_code;
}
int tns_314_SNS(uchar *pkt, uint len, void *userdata)
{
    int err_code = CODE_SUCCESS;
    uchar   client_ver[4] = {0};
    uchar   server_ver[4] = {0};
    struct tns_session *sess = (struct tns_session*)userdata;

    if (len <= 0)
        goto err;

    if (sess->cur_dir == REQUEST)
    {
        memcpy(client_ver, pkt+8, sizeof(client_ver));
        printf("client version: %d.%d.%d.%d\n",client_ver[0],client_ver[1],client_ver[2],client_ver[3]);
    }
    else
    {
        memcpy(server_ver, pkt+8, sizeof(server_ver));
        printf("server version: %d.%d.%d.%d\n",server_ver[0],server_ver[1],server_ver[2],server_ver[3]);
    }
    
err:
    return err_code;
}


int tns_connect_handle(uchar *pkt, uint len, void *userdata)
{
    int err_code = CODE_SUCCESS;
    int outlen = 0;
    char *jsonbuf = NULL;
    cJSON *root = NULL;
    cJSON *value = NULL;
    cJSON *desc = NULL;
    cJSON *conn_data = NULL;
    cJSON *cid = NULL;
    struct tns_session *sess = (struct tns_session*)userdata;
    
    if (len <= TNS_HEAD_LEN)
        goto err;
    
    debug_hex_println("dump tns connect", PKT_DATA(pkt), len - TNS_HEAD_LEN);
    
    if (0 >= CONNECT_DATA_LEN(pkt))
        goto err;

    jsonbuf = (char*)malloc(2 * CONNECT_DATA_LEN(pkt));
    outlen = 2 * CONNECT_DATA_LEN(pkt);

    sess->version = CONNECT_VER(pkt);
    sess->version_compatible = CONNECT_VER_COMPATIBLE(pkt);
    
    //fsm_set_state(sess, STAT_CONNECT);
    sess->state = STAT_CONNECT;
    
    parse_connect_data_to_json((char*)CONNECT_DATA(pkt), CONNECT_DATA_LEN(pkt), jsonbuf, &outlen);

    printf("%s %s connect data len:%d\n", sess->cur_sess_desc, direction_str[sess->cur_dir], outlen);

    root = cJSON_Parse(jsonbuf);
    if (root == NULL)
        goto err;
    
    desc = cJSON_GetObjectItem(root, "DESCRIPTION");
    if (desc == NULL)
        goto err;
    
    conn_data = cJSON_GetObjectItem(desc, "CONNECT_DATA");
    if (desc == NULL)
        goto err;
    
    value = cJSON_GetObjectItem(conn_data, "SERVICE_NAME");
    if (value != NULL && cJSON_String == value->type)
        printf("    SERVICE_NAME : %s\n", value->valuestring);
    
    cid = cJSON_GetObjectItem(conn_data, "CID");
    if (desc == NULL)
        goto err;
    
    value = cJSON_GetObjectItem(cid, "HOST");
    if (value != NULL && cJSON_String == value->type)
        printf("    HOST : %s\n", value->valuestring);
    
    value = cJSON_GetObjectItem(cid, "USER");
    if (value != NULL && cJSON_String == value->type)
        printf("    USER : %s\n", value->valuestring);

    //record link state connect or resend
err:
    return err_code;
}

int tns_accept_handle(uchar *pkt, uint len, void *userdata)
{
    int err_code = CODE_SUCCESS;
    struct tns_session *sess = (struct tns_session*)userdata;
    
    if (len <= TNS_HEAD_LEN)
        goto err;
    
    debug_hex_println("dump tns accept", PKT_DATA(pkt), len - TNS_HEAD_LEN);

    sess->version = ACCEPT_VER(pkt);

    //fsm_set_state(sess, STAT_ACCEPT);
    sess->state = STAT_ACCEPT;
    
    if (0 >= ACCEPT_DATA_LEN(pkt))
        goto err;

    //todo record state link success
err:
    return err_code;
}

int tns_refuse_handle(uchar *pkt, uint len, void *userdata)
{
    int err_code = CODE_SUCCESS;
    struct tns_session *sess = (struct tns_session*)userdata;

    if (len <= TNS_HEAD_LEN)
        goto err;
    
    debug_hex_println("dump tns refuse", PKT_DATA(pkt), len - TNS_HEAD_LEN);

    //fsm_set_state(sess, STAT_REFUSE);
    sess->state = STAT_REFUSE;

    if (0 >= REFUSE_DATA_LEN(pkt))
        goto err;
    //todo record state link refuse
err:
    return err_code;
}

int tns_data_handle(uchar *pkt, uint len, void *userdata)
{
    int err_code = CODE_SUCCESS;
    struct tns_session *sess = (struct tns_session*)userdata;

    if (len <= TNS_HEAD_LEN)
        goto err;
    
    debug_hex_println("dump tns data", PKT_DATA(pkt), len - TNS_HEAD_LEN);

    //fsm_set_state(sess, STAT_DATA);
    sess->state = STAT_DATA;

    if (len <= (TNS_HEAD_LEN + OFFSET_TNS_DATA_OFFSET))
        goto err;
    
    //msg data id handle
    /*
    printf("%s %s tns data : version :%d, dataid: %02x\n",
                    sess->cur_sess_desc,
                    direction_str[sess->cur_dir],
                    sess->version,
                    DATA_ID(pkt));
    */
    msg_data_id_handle(msg_data_id_handles,
                            sizeof(msg_data_id_handles)/sizeof(struct Pkt_handle),
                            sess->version,
                            DATA_ID(pkt),
                            PKT_DATA(pkt),
                            len,
                            userdata);
err:
    return err_code;
}

int tns_resend_handle(uchar *pkt, uint len, void *userdata)
{
    int err_code = CODE_SUCCESS;
    if (len <= TNS_HEAD_LEN)
        goto err;
    //debug_hex_println("dump tns resend", PKT_DATA(pkt), len - TNS_HEAD_LEN);
err:
    return err_code;
}

int msg_data_id_handle(struct Pkt_handle ph[], int num, ushort ver, uchar type, uchar *data, uint len, void *userdata)
{
    int i = 0, err_code = CODE_SUCCESS;
    
    for (; i < num; i++)
    {
        if (ph[i].version == ver && ph[i].type == type && ph[i].func != NULL)
        {
            err_code = ph[i].func(data,len,userdata);
            break;
        }
    }
    if (i == num)
    {
        err_code = CODE_DATA_ID;
        goto err;
    }
err:
    return err_code;
}

int packet_handle(struct Pkt_handle ph[], int num, uchar type, uchar *data, uint len, void *userdata)
{
    int i = 0, err_code = CODE_SUCCESS;
    
    for (; i < num; i++)
    {
        if (ph[i].type == type && ph[i].func != NULL)
        {
            err_code = ph[i].func(data,len,userdata);
            break;
        }
    }
    if (i == num)
    {
        err_code = CODE_PKT_TYPE;
        goto err;
    }
err:
    return err_code;
}

int TNS_protocol_parse(uchar *data, uint len, void *userdata)
{
    int err_code = CODE_SUCCESS;
    
    if (len != PKT_LEN(data))
    {
        err_code = CODE_PKT_LEN;
        goto err; 
    }

    return packet_handle(msg_type_handles,
                            sizeof(msg_type_handles)/sizeof(struct Pkt_handle),
                            PKT_TYPE(data),
                            data,
                            len,
                            userdata);
err:
    return err_code;
}

