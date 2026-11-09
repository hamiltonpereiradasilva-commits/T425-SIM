#include <stdio.h>
#include <string.h>
#include "server.h"
#include "pack.h"
#include "files.h"
#include "sp.h"

unsigned char Tbuf[TRANSACTION_BUFFER_SIZE];

static FILE *test_stdout;

void dbgmsg() { }
unsigned long long sp_reply_diag_current(void) { return 1; }
int sp_reply_diag_is_puts(void) { return 0; }
void sp_reply_diag_record(const char *stage, unsigned long long txn, int rc,
                          const void *tbuf, const void *outbuf, const void *inbuf,
                          int outcount, int calculated_length, const void *argbuf)
{ (void)stage; (void)txn; (void)rc; (void)tbuf; (void)outbuf; (void)inbuf;
  (void)outcount; (void)calculated_length; (void)argbuf; }
void sp_reply_diag_put_count(const char *stage, unsigned long long txn,
                             unsigned int len, const void *len_address,
                             const void *tbuf, const void *outbuf, int outcount)
{ (void)stage; (void)txn; (void)len; (void)len_address; (void)tbuf;
  (void)outbuf; (void)outcount; }

static int check_get32(const unsigned char wire[4], unsigned long expected)
{
   unsigned long actual;
   InBuf = (unsigned char *)wire;
   unsigned char *before = InBuf;
   actual = get_32();
   if (actual != expected || InBuf != before + 4) {
      fprintf(stderr, "get_32 failed: expected=%lu actual=%lu consumed=%ld\n",
              expected, actual, (long)(InBuf - before));
      return 0;
   }
   return 1;
}

void SpPuts();

int main(void)
{
   static const struct { unsigned char b[4]; unsigned long value; } cases[] = {
      {{0x01,0x00,0x00,0x00}, 1UL},
      {{0x00,0x00,0x00,0x00}, 0UL},
      {{0x7f,0x00,0x00,0x00}, 127UL},
      {{0x80,0x00,0x00,0x00}, 128UL},
      {{0xff,0xff,0xff,0xff}, 0xffffffffUL}
   };
   unsigned int i;
   int status;
   char output[64];
   size_t n;

   for (i=0; i<sizeof(cases)/sizeof(cases[0]); i++)
      if (!check_get32(cases[i].b, cases[i].value)) return 1;

   /* A complete real SP_PUTS request body for stdout=1 and 14 text bytes. */
   memset(Tbuf, 0, sizeof(Tbuf));
   Tbuf[0]=0x16; Tbuf[1]=0x00; Tbuf[2]=SP_PUTS;
   Tbuf[3]=0x01; Tbuf[4]=0x00; Tbuf[5]=0x00; Tbuf[6]=0x00;
   Tbuf[7]=0x0e; Tbuf[8]=0x00;
   memcpy(&Tbuf[9], "Hello world...", 14);
   test_stdout=tmpfile();
   if (test_stdout == NULL) { perror("tmpfile"); return 2; }
   memset(&FileInfo[FD_STDOUT], 0, sizeof(FileInfo[FD_STDOUT]));
   FileInfo[FD_STDOUT].fd=test_stdout;
   FileInfo[FD_STDOUT].lastop=FIOP_NONE;
   InBuf=&Tbuf[3]; OutBuf=&Tbuf[2]; OutCount=0;
   SpPuts();
   status=Tbuf[2];
   if (status == ER_BADID || status != ER_SUCCESS) {
      fprintf(stderr, "SP_PUTS failed: status=0x%02x\n", status);
      return 3;
   }
   if (Tbuf[0] != 2 || Tbuf[1] != 0 || OutCount != 1) {
      fprintf(stderr, "SP_PUTS reply header/count mismatch: %02x %02x count=%d\n",
              Tbuf[0], Tbuf[1], OutCount);
      return 4;
   }
   fflush(test_stdout); rewind(test_stdout);
   n=fread(output,1,sizeof(output)-1,test_stdout); output[n]='\0';
   if (strcmp(output, "Hello world...\n") != 0) {
      fprintf(stderr, "SP_PUTS output mismatch: <%s>\n", output);
      return 5;
   }
   puts("PASS get_32 little-endian vectors and exact 4-byte advancement");
   puts("PASS SP_PUTS stdout descriptor=1, ER_SUCCESS, text emitted, reply length=2");
   fclose(test_stdout);
   return 0;
}
