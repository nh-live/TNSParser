#include "common.h"
#include "util.h"

int g_hexdump_enbale = 0;

int is_hexdump_enable()
{
    return g_hexdump_enbale;
}

static void print_ansi_at_line(void *data, uint len)
{
	uint i = 0;
	uchar *ptr = (uchar*)data;
	for (; i < len; i++)
	{
		if (ptr[i] >= 0x20 && ptr[i] <= 0x7e)
			printf("%c",ptr[i]);
		else
			printf(".");
	}
}

void debug_hex_println(char *begin, void* data, uint len)
{
    uint i = 0, tmp = 0;
    uchar *ptr = (uchar*)data;

    if (!is_hexdump_enable())
        return;
    
    printf("\n%s----start---- len %u\n", begin, len);
    for (; i < len; i++)
    {
        printf("%02x ", ptr[i]);
        if ((i + 1) % 32 == 0) {
	    print_ansi_at_line(&ptr[i-31],32);
		printf("\n");
        }
    }
    for (tmp = i; tmp%32 != 0; tmp++)
		printf("   ");
    if (i != 0)
        print_ansi_at_line(&ptr[i-i%32],i%32+1);
    printf("\n%s----end----\n\n", begin);
}

