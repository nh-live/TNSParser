#include "util.h"
#include "process.h"
#include "oracle_tns.h"

int mainprocess(uchar *data, uint *len)
{
    int ret = 0;
    debug_hex_println("dump mainprocess data", data, *len);

    ret = TNS_protocol_parse(data, *len);
    
    return ret;
}

