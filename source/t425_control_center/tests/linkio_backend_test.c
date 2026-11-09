#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "../iserver/linkusb_linkio.h"

enum { READ_FULL, READ_1_PLUS_6, READ_2_PLUS_5, READ_ONE_BYTE, READ_TIMEOUT_EMPTY,
       READ_TIMEOUT_PARTIAL, READ_FATAL, READ_DEADLINE };
static const unsigned char expected[] = {0x01,0x00,0x02,0x00,0xA5,0x5A,0x11,0x22};
static unsigned char source[sizeof(expected)];
static unsigned int source_n, source_at;
static int read_mode, read_calls, read_timeouts[64];
static int sent[64], sent_n, write_calls, usb_accepted, fail_at=-1;

static unsigned long long monotonic_us(void) {
    struct timespec ts;
    assert(clock_gettime(CLOCK_MONOTONIC, &ts) == 0);
    return (unsigned long long)ts.tv_sec * 1000000ULL + (unsigned long long)ts.tv_nsec / 1000ULL;
}

static int fake_open(const char *resource) { return strcmp(resource,"linkusb0")==0 ? 0 : -1; }
static int fake_close(int id) { return id==0 ? 0 : -1; }
static int fake_write(unsigned char b, int timeout) {
    assert(timeout > 0);
    ++write_calls;
    ++usb_accepted;
    if (write_calls == fail_at) return 0;
    sent[sent_n++] = b;
    return 1;
}
static int fake_read(char *buf, unsigned int count, int timeout) {
    unsigned int n;
    assert(timeout > 0 && read_calls < 64);
    read_timeouts[read_calls] = timeout;
    ++read_calls;
    if (read_mode == READ_TIMEOUT_EMPTY) { usleep((useconds_t)timeout * 1000u); return 0; }
    if (read_mode == READ_FATAL && read_calls == 2) return -1;
    if (read_mode == READ_TIMEOUT_PARTIAL && source_at >= 2) {
        usleep((useconds_t)timeout * 1000u);
        return 0;
    }
    if (source_at >= source_n) return 0;
    switch (read_mode) {
      case READ_1_PLUS_6: n = read_calls == 1 ? 1u : 6u; break;
      case READ_2_PLUS_5: n = read_calls == 1 ? 2u : 5u; break;
      case READ_ONE_BYTE: case READ_DEADLINE: n = 1u; break;
      case READ_TIMEOUT_PARTIAL: n = 2u; break;
      case READ_FATAL: n = 1u; break;
      default: n = count; break;
    }
    if (n > count) n = count;
    if (n > source_n - source_at) n = source_n - source_at;
    if (read_mode == READ_DEADLINE) {
        if (timeout < 30) { usleep((useconds_t)timeout * 1000u); return 0; }
        usleep(30000);
    }
    memcpy(buf, source + source_at, n);
    source_at += n;
    return (int)n;
}
static int fake_ready_read(void) { return source_at < source_n ? 1 : 0; }
static int fake_ready_write(void) { return 1; }
static void reset_read(int mode) {
    memcpy(source, expected, sizeof(expected));
    source_n = sizeof(expected); source_at = 0; read_mode = mode; read_calls = 0;
    memset(read_timeouts, 0, sizeof(read_timeouts));
}
static void exact_case(int mode, const char *label, unsigned int fragments, unsigned int count) {
    char out[sizeof(expected)] = {0};
    reset_read(mode);
    assert(ReadLink(0, out, count, 100) == (int)count);
    assert(memcmp(out, expected, count) == 0);
    assert(source_at == count && read_calls == (int)fragments);
    printf("%s: PASS bytes=%u fragments=%d order=preserved\n", label, count, read_calls);
}

int main(void) {
    t425_linkio_ops_t ops={fake_open,fake_close,fake_write,fake_read,fake_ready_read,fake_ready_write};
    char buf[]="hello", out[16]={0};
    t425_linkio_install_test_ops(&ops);
    assert(OpenLink("linkusb0")==0);

    exact_case(READ_FULL, "complete_transfer", 1, 8);
    exact_case(READ_1_PLUS_6, "fragment_1_plus_6", 2, 7);
    exact_case(READ_2_PLUS_5, "fragment_2_plus_5", 2, 7);
    exact_case(READ_ONE_BYTE, "one_byte_per_transfer", 8, 8);

    reset_read(READ_TIMEOUT_EMPTY);
    {
      unsigned long long start=monotonic_us(), end; int result;
      result=ReadLink(0,out,8,1); end=monotonic_us();
      assert(result==0 && read_calls==1);
      printf("timeout_without_bytes: PASS requested_ms=100 effective_us=%llu transfers=%d accumulated=0 result=TIMEOUT\n", end-start, read_calls);
    }

    reset_read(READ_TIMEOUT_PARTIAL);
    {
      unsigned long long start=monotonic_us(), end; int result;
      result=ReadLink(0,out,8,1); end=monotonic_us();
      assert(result==2);
      printf("timeout_partial: PASS requested_ms=100 effective_us=%llu transfers=%d accumulated=%d result=PARTIAL\n", end-start, read_calls, result);
    }
    assert(memcmp(out, expected, 2)==0 && source_at==2);
    puts("timeout_after_partial_progress: PASS exact_prefix=YES");

    reset_read(READ_FATAL);
    assert(ReadLink(0, out, 8, 1)==-1);
    assert(source_at==1 && memcmp(out, expected, 1)==0);
    puts("fatal_error_after_fragment: PASS result=-1");

    reset_read(READ_DEADLINE);
    {
      unsigned long long start=monotonic_us(), end; int result;
      result=ReadLink(0, out, 8, 1);
      end=monotonic_us();
      assert(result==3);
      assert(end-start < 250000ULL);
      assert(read_calls >= 3 && read_timeouts[0] >= read_timeouts[read_calls-1]);
      assert(memcmp(out, expected, 3)==0);
      printf("absolute_deadline: PASS requested_ms=100 effective_us=%llu transfers=%d accumulated=3 result=PARTIAL timeout_nonincreasing=YES\n",
             end-start, read_calls);
    }

    reset_read(READ_FULL);
    assert(ReadLink(0, out, 0, 1)==0 && read_calls==0);
    assert(ReadLink(0, out, 8, 0)==-1);
    assert(ReadLink(0, NULL, 8, 1)==-1);
    assert(ReadLink(0, out, 8, 1)==8 && memcmp(out,expected,8)==0);
    puts("zero_count_and_argument_bounds: PASS");

    reset_read(READ_FULL);
    assert(ReadLink(0,out,4,100)==4 && source_at==4);
    assert(ReadLink(0,out+4,4,100)==4 && source_at==8);
    assert(memcmp(out,expected,8)==0);
    puts("no_overread_or_loss_between_calls: PASS");

    assert(WriteLink(0,buf,5,100)==5);
    assert(sent_n==5 && sent[0]=='h' && sent[1]=='e' && sent[2]=='l' && sent[3]=='l' && sent[4]=='o');
    {
      int accepted_before=usb_accepted, acked_before=sent_n, calls_before=write_calls;
      fail_at=write_calls+2;
      assert(WriteLink(0,buf,4,100)==1);
      assert(usb_accepted-accepted_before==2 && sent_n-acked_before==1 && write_calls-calls_before==2);
    }
    fail_at=write_calls+1;
    assert(WriteLink(0,buf,1,100)==-1);
    reset_read(READ_FULL);
    assert(TestRead(0)==1 && TestWrite(0)==1);
    assert(TestError(0)==-1 && ResetLink(0)==-1 && AnalyseLink(0)==-1);
    assert(CloseLink(0)==0);
    puts("META_WriteLink_and_existing_contracts: PASS");
    return 0;
}
