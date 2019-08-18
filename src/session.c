#include "session.h"

struct tns_session g_sess_arry[MAX_SESS_NODE];

/*
* Find sess from the array.
* If the array is full, reset the oldest session and return his pointer.
*/
struct tns_session* sess_must_find_node(struct sess_key skey, int dir)
{
    int i = 0;
    int empty_node_index = 0;
    ulonglong cur_time = 0;
    ulonglong diff_time = 0;
    struct tns_session *tmp = NULL;
    struct sess_key key;

    cur_time = time(NULL);
    
    if (REQUEST == dir)
    {
        key.sip = skey.sip;
        key.dip = skey.dip;
        key.sport = skey.sport;
        key.dport = skey.dport;
    }
    else
    {
        key.sip = skey.dip;
        key.dip = skey.sip;
        key.sport = skey.dport;
        key.dport = skey.sport;
    }

    // find sess
    for (i = 0; i < MAX_SESS_NODE; i++)
    {
        tmp = &g_sess_arry[i];
        if ((tmp->key.sip == key.sip) && (tmp->key.dip == key.dip) && 
            (tmp->key.sport == key.sport) && (tmp->key.dport == key.dport))
        {
            tmp->update_time = time(NULL);
            break;
        }
    }

    // get a empty sess and set the key
    if (MAX_SESS_NODE == i)
    {
        for (i = 0; i < MAX_SESS_NODE; i++)
        {
            tmp = &g_sess_arry[i];
            if (tmp->state == STAT_CLOSE);
            {
                memcpy(&tmp->key,&key,sizeof(tmp->key));
                tmp->state = STAT_INIT;
                break;
            }
            
            if (cur_time - tmp->update_time > diff_time)
            {
                empty_node_index = i;
                diff_time = cur_time - tmp->update_time;
            }
        }
    }

    // reset oldest sess
    if (MAX_SESS_NODE == i)
    {
        memset(&g_sess_arry[i], 0, sizeof(struct tns_session));
        memcpy(&tmp->key,&key,sizeof(tmp->key));
        tmp = &g_sess_arry[empty_node_index];
        tmp->state = STAT_INIT;
    }

    return tmp;    
}

int sess_init(void)
{
    int i = 0;

    for (; i < 32; i++)
    {
        memset(&g_sess_arry[i], 0, sizeof(struct tns_session));
    }
    
    return 0;
}

