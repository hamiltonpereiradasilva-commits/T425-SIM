#include <stdio.h>
#include <stdlib.h>
#include "server.h"
#include "iserver.h"
#include "pack.h"

unsigned char Tbuf[TRANSACTION_BUFFER_SIZE];
unsigned char *InBuf;
unsigned char *OutBuf;
int OutCount;

int sp_reply_diag_is_puts(void) { return 0; }
unsigned long long sp_reply_diag_current(void) { return 0; }
void sp_reply_diag_put_count(const char *stage, unsigned long long txn,
                             unsigned int len, const void *len_address,
                             const void *tbuf, const void *outbuf, int outcount)
{ (void)stage; (void)txn; (void)len; (void)len_address; (void)tbuf; (void)outbuf; (void)outcount; }

int main(void)
{
    static const unsigned int lengths[] = {0, 1, 2, 3, 255, 256, 257, 65534, 65535};
    unsigned int i;
    for (i = 0; i < sizeof(lengths) / sizeof(lengths[0]); ++i) {
        unsigned int rounded = lengths[i] + (lengths[i] & 1U);
        unsigned int encoded = rounded & 0xFFFFU;
        Tbuf[0] = Tbuf[1] = 0xA5;
        put_count(lengths[i]);
        if (Tbuf[0] != (unsigned char)(encoded & 0xFFU) ||
            Tbuf[1] != (unsigned char)((encoded >> 8) & 0xFFU)) {
            fprintf(stderr, "FAIL len=%u got=%02X %02X expected=%02X %02X\n",
                    lengths[i], Tbuf[0], Tbuf[1], encoded & 0xFFU, encoded >> 8);
            return 1;
        }
        printf("PASS len=%u rounded=%u wire=%02X %02X\n",
               lengths[i], rounded, Tbuf[0], Tbuf[1]);
    }
    Tbuf[2] = 0x87;
    put_count(1);
    if (Tbuf[0] != 0x02 || Tbuf[1] != 0x00 || Tbuf[2] != 0x87) {
        fprintf(stderr, "FAIL SP_PUTS_PREFIX=%02X %02X %02X\n", Tbuf[0], Tbuf[1], Tbuf[2]);
        return 1;
    }
    puts("PASS SP_PUTS_PREFIX=02 00 87 (status byte preserved, semantics unchanged)");
    return 0;
}
