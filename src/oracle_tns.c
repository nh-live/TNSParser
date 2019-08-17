#include "oracle_tns.h"

struct Pkt_handle msg_type_handles[] =
{
    {TNS_TYPE_CONNECT,  tns_connect_handle},
    {TNS_TYPE_ACCEPT,   tns_accept_handle},
    {TNS_TYPE_ACK,      NULL},
    {TNS_TYPE_REFUSE,   tns_refuse_handle},
    {TNS_TYPE_REDIRECT, NULL},
    {TNS_TYPE_DATA,     tns_data_handle},
    {TNS_TYPE_NULL,     NULL},
    {TNS_TYPE_UNKNOWN1,  NULL},
    {TNS_TYPE_ABORT,    NULL},
    {TNS_TYPE_UNKNOWN2,  NULL},
    {TNS_TYPE_RESEND,   tns_resend_handle},
    {TNS_TYPE_MARKER,   NULL},
    {TNS_TYPE_UNKNOWN3,  NULL},
    {TNS_TYPE_UNKNOWN4,  NULL},
};

struct Pkt_handle msg_data_id_handles[] = 
{
    {TNS_DATA_ID_SNS,   tns_313_SNS},   //secure network service
    {TNS_DATA_ID_SP,    tns_313_SP},    //set protocol
    {TNS_DATA_ID_OCI,   tns_313_OCI},   //OCI function
    {TNS_DATA_ID_RS,    tns_313_RS}     //return status
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

int parse_continuous_visible_characters(char *bufin, int inlen, char **ptr, int *outlen)
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
    *outlen = continuous_times;
    return 0;
}

int tns_313_RS(uchar *pkt, uint len, void *userdata)
{
    int err_code = CODE_SUCCESS;
    char *ptr_sqlresult = NULL;
    int result_len = 0;
    
    if (len <= 0)
        goto err;
    //if (*(pkt+3) == 0x5e)
    {
        parse_continuous_visible_characters((char*)pkt, len, &ptr_sqlresult, &result_len);
        printf(" return status : %d\n%s\n",result_len,ptr_sqlresult);
    }
err:
    return err_code;
}

int tns_313_OCI(uchar *pkt, uint len, void *userdata)
{
    int err_code = CODE_SUCCESS;
    int sql_len = 0;
    char *ptr_sqlstart = NULL;
    
    if (len <= 0)
        goto err;
    
    //todo extra sql
    if (*(pkt+3) == 0x5e)
    {
        parse_continuous_visible_characters((char*)pkt, len, &ptr_sqlstart, &sql_len);
        printf(" sql : %d\n%s\n",sql_len,ptr_sqlstart);
    }
err:
    return err_code;
}
int tns_313_SP(uchar *pkt, uint len, void *userdata)
{
    int err_code = CODE_SUCCESS;
    
    if (len <= 0)
        goto err;
    
    printf("313_client platform %s\n",pkt+3+(strlen((char*)pkt+3)+1));
err:
    return err_code;
}
int tns_313_SNS(uchar *pkt, uint len, void *userdata)
{
    int err_code = CODE_SUCCESS;
    uchar   client_ver[4] = {0};

    if (len <= 0)
        goto err;

    memcpy(client_ver, pkt+8, sizeof(client_ver));
    printf("313_client ver %d.%d.%d.%d\n",client_ver[0],client_ver[1],client_ver[2],client_ver[3]);
err:
    return err_code;
}

int tns_connect_handle(uchar *pkt, uint len, void *userdata)
{
    int err_code = CODE_SUCCESS;
    char *jsonbuf = NULL;
    int outlen = 0;
    
    if (len <= TNS_HEAD_LEN)
        goto err;
    debug_hex_println("dump tns connect", PKT_DATA(pkt), len - TNS_HEAD_LEN);
    if (0 >= CONNECT_DATA_LEN(pkt))
        goto err;
    debug_hex_println("dump tns connect data", CONNECT_DATA(pkt), CONNECT_DATA_LEN(pkt));

    jsonbuf = (char*)malloc(2 * CONNECT_DATA_LEN(pkt));
    outlen = 2 * CONNECT_DATA_LEN(pkt);
    parse_connect_data_to_json((char*)CONNECT_DATA(pkt), CONNECT_DATA_LEN(pkt), jsonbuf, &outlen);
    printf("\nconnect data to json : %d\n%s\n",outlen, jsonbuf);

    //record link state connect or resend
err:
    return err_code;
}

int tns_accept_handle(uchar *pkt, uint len, void *userdata)
{
    int err_code = CODE_SUCCESS;
    
    if (len <= TNS_HEAD_LEN)
        goto err;
    debug_hex_println("dump tns accept", PKT_DATA(pkt), len - TNS_HEAD_LEN);
    if (0 >= ACCEPT_DATA_LEN(pkt))
        goto err;
    //debug_hex_println("dump tns accept data", ACCEPT_DATA(pkt), ACCEPT_DATA_LEN(pkt));

    //todo record state link success
err:
    return err_code;
}

int tns_refuse_handle(uchar *pkt, uint len, void *userdata)
{
    int err_code = CODE_SUCCESS;
    
    if (len <= TNS_HEAD_LEN)
        goto err;
    debug_hex_println("dump tns refuse", PKT_DATA(pkt), len - TNS_HEAD_LEN);
    if (0 >= REFUSE_DATA_LEN(pkt))
    debug_hex_println("dump tns refuse data", REFUSE_DATA(pkt), REFUSE_DATA_LEN(pkt));

    //todo record state link refuse
err:
    return err_code;
}

int tns_data_handle(uchar *pkt, uint len, void *userdata)
{
    int err_code = CODE_SUCCESS;
    
    if (len <= TNS_HEAD_LEN)
        goto err;
    debug_hex_println("dump tns data", PKT_DATA(pkt), len - TNS_HEAD_LEN);
    if (len <= (TNS_HEAD_LEN + OFFSET_TNS_DATA_OFFSET))
        goto err;
    
    //msg data id handle
    printf("%02x \n",DATA_ID(pkt));
    msg_data_id_handle(msg_data_id_handles,
                            sizeof(msg_data_id_handles)/sizeof(struct Pkt_handle),
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
    debug_hex_println("dump tns resend", PKT_DATA(pkt), len - TNS_HEAD_LEN);
err:
    return err_code;
}

int msg_data_id_handle(struct Pkt_handle ph[], int num, uchar type, uchar *data, uint len, void *userdata)
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

int TNS_protocol_parse(uchar *data, uint len)
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
                            NULL);
err:
    return err_code;
}

