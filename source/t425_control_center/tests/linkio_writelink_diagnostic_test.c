#define _POSIX_C_SOURCE 200809L
#include "linkusb_linkio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

extern void t425_linkio_install_write_diag_hooks(
    int (*)(unsigned char out[56]),
    int (*)(unsigned char, unsigned char *, int, int *, int),
    unsigned long long (*)(void));
extern void t425_linkio_test_set_open(int);

enum branch {
    B4_FAIL, WAIT_ACK, QUEUE_BUSY, BULK_ERROR, BULK_PARTIAL, ACK_TIMEOUT, COMPLETE
};
static enum branch target;
static int after_progress;
static int b4_calls, bulk_calls;
static unsigned char sent_bytes[4];
static unsigned long long clock_ms;

static int fake_b4(unsigned char out[56]) {
    int call = ++b4_calls;
    uint32_t ack = 0;
    memset(out, 0, 56);
    if ((target == B4_FAIL) && call == (after_progress ? 3 : 1)) return -1;
    if (target == WAIT_ACK && call == (after_progress ? 3 : 1)) out[6] |= 2;
    if (target == QUEUE_BUSY && call == (after_progress ? 3 : 1)) out[40] = 1;
    if (after_progress && call >= 2) ack = 1;
    if (call == 2 && (target != ACK_TIMEOUT || after_progress)) ack = 1;
    if (target == COMPLETE && after_progress && call >= 4) ack = 2;
    out[20] = (unsigned char)(ack & 0xff);
    out[21] = (unsigned char)((ack >> 8) & 0xff);
    out[22] = (unsigned char)((ack >> 16) & 0xff);
    out[23] = (unsigned char)((ack >> 24) & 0xff);
    return 0;
}

static int fake_bulk(unsigned char ep, unsigned char *data, int length, int *transferred, int timeout_ms) {
    (void)data;
    assert(ep == 0x02 && length == 1 && timeout_ms == 100);
    sent_bytes[bulk_calls++] = data[0];
    *transferred = 1;
    if (target == BULK_ERROR && bulk_calls == (after_progress ? 2 : 1)) return -4;
    if (target == BULK_PARTIAL && bulk_calls == (after_progress ? 2 : 1)) { *transferred = 0; return 0; }
    return 0;
}

static unsigned long long fake_clock(void) { unsigned long long value=clock_ms; clock_ms+=10; return value; }

static const char *cause_for(enum branch b) {
    switch (b) {
        case B4_FAIL: return "B4_READ_FAILED";
        case WAIT_ACK: return "TX_WAITING_ACK";
        case QUEUE_BUSY: return "TX_QUEUE_BUSY";
        case BULK_ERROR: return "LIBUSB_BULK_ERROR";
        case BULK_PARTIAL: return "LIBUSB_PARTIAL_TRANSFER";
        case ACK_TIMEOUT: return "ACK_TIMEOUT";
        case COMPLETE: return "WRITE_COMPLETED";
    }
    return "";
}

static void run_case(enum branch b, int progress) {
    FILE *capture = tmpfile();
    int saved, result, expected, count = progress ? 2 : 1;
    char log[2048]; size_t n;
    char payload[2] = { (char)0xF0, (char)0xA5 };
    assert(capture);
    target=b; after_progress=progress; b4_calls=bulk_calls=0; clock_ms=1000;
    memset(sent_bytes,0,sizeof(sent_bytes));
    t425_linkio_test_set_open(1);
    t425_linkio_install_write_diag_hooks(fake_b4, fake_bulk, fake_clock);
    saved=dup(STDERR_FILENO); assert(saved>=0);
    fflush(stderr); assert(dup2(fileno(capture), STDERR_FILENO)>=0);
    result=WriteLink(0, payload, (unsigned int)count, 1);
    fflush(stderr); assert(dup2(saved, STDERR_FILENO)>=0); close(saved);
    rewind(capture); n=fread(log,1,sizeof(log)-1,capture); log[n]='\0'; fclose(capture);
    expected = b==COMPLETE ? count : (progress ? 1 : -1);
    assert(result==expected);
    assert(strstr(log,"[LINKIO_WRITE_DIAG]") && strstr(log,cause_for(b)));
    assert(strchr(log,'\n') && strchr(log,'\n')==strrchr(log,'\n'));
    assert(strstr(log,"count=") && strstr(log,"timeout_ms=100") && strstr(log,"pid="));
    if (progress) assert(strstr(log,b==COMPLETE ? "done=2" : "done=1"));
    if (b==BULK_ERROR) assert(strstr(log,"libusb_rc=-4"));
    if (b==BULK_PARTIAL) assert(strstr(log,"transferred=0"));
    if (b==ACK_TIMEOUT) assert(strstr(log,"deadline_ms="));
    if (b==COMPLETE) assert(strstr(log,"cause=WRITE_COMPLETED"));
    if (bulk_calls > 0) assert(sent_bytes[0]==(unsigned char)payload[0]);
    if (bulk_calls > 1) assert(sent_bytes[1]==(unsigned char)payload[1]);
    printf("%s progress=%d: PASS return=%d bulk_calls=%d b4_calls=%d\n",cause_for(b),progress,result,bulk_calls,b4_calls);
}

int main(void) {
    run_case(B4_FAIL,0); run_case(B4_FAIL,1);
    run_case(WAIT_ACK,0); run_case(WAIT_ACK,1);
    run_case(QUEUE_BUSY,0); run_case(QUEUE_BUSY,1);
    run_case(BULK_ERROR,0); run_case(BULK_ERROR,1);
    run_case(BULK_PARTIAL,0); run_case(BULK_PARTIAL,1);
    run_case(ACK_TIMEOUT,0); run_case(ACK_TIMEOUT,1);
    run_case(COMPLETE,0); run_case(COMPLETE,1);
    t425_linkio_install_write_diag_hooks(NULL,NULL,NULL);
    t425_linkio_test_set_open(0);
    return 0;
}
