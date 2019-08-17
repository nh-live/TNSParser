#include "common.h"
#include "util.h"

static void debug_ansi_at_line(void *data, uint len)
{
	uint m = 0;
	uchar *ptr = (uchar*)data;
	for (; m < len; m++)
	{
		if (ptr[m] >= 0x20 && ptr[m] <= 0x7e)
			printf("%c",ptr[m]);
		else
			printf(".");
	}
}

void debug_hex_println(char *begin, void* data, uint len)
{
    uint m = 0, tmp = 0;
    uchar* ptr = (uchar*)data;

    printf("\n%s----start---- len %u\n", begin, len);
    for (; m < len; m++)
    {
        printf("%02x ", ptr[m]);
        if ((m + 1) % 32 == 0) {
	    debug_ansi_at_line(&ptr[m-31],32);
		printf("\n");
        }
    }
    for (tmp = m; tmp%32 != 0; tmp++)
		printf("   ");
    if (m != 0)
        debug_ansi_at_line(&ptr[m-m%32],m%32+1);
    printf("\n%s----end----\n\n", begin);
}

