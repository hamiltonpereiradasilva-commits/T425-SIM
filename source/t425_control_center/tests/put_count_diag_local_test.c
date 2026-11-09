#include <stdio.h>
#include "server.h"
#include "iserver.h"
#include "sp_reply_diag.h"

unsigned char Tbuf[TRANSACTION_BUFFER_SIZE];
extern int OutCount;
extern void put_count();

int main(void)
{
    OutCount = 1;
    Tbuf[0] = Tbuf[1] = Tbuf[2] = Tbuf[3] = 0;
    sp_reply_diag_begin(1);
    put_count(OutCount);
    printf("LOCAL_RESULT len=1 header=%02X %02X tail=%02X %02X\n",
           Tbuf[0], Tbuf[1], Tbuf[2], Tbuf[3]);
    return (Tbuf[0] == 0x02 && Tbuf[1] == 0x00) ? 0 : 1;
}
