#include "sp_reply_diag.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static volatile unsigned long long event_sequence = 0;
static volatile unsigned long long transaction_sequence = 0;
static volatile unsigned long long current_transaction = 0;
static volatile int current_is_sp_puts = 0;
static volatile unsigned long long read_call_sequence = 0;

static unsigned long long increment(volatile unsigned long long *value)
{
    return __sync_add_and_fetch(value, 1ULL);
}

unsigned long long sp_reply_diag_begin(int is_sp_puts)
{
    unsigned long long id = increment(&transaction_sequence);
    current_transaction = id;
    current_is_sp_puts = is_sp_puts ? 1 : 0;
    return id;
}

unsigned long long sp_reply_diag_current(void)
{
    return current_transaction;
}

int sp_reply_diag_is_puts(void)
{
    return current_is_sp_puts;
}

static void dump8(const unsigned char *p)
{
    int i;
    if (p == NULL) {
        fputs("null", stderr);
        return;
    }
    for (i = 0; i < 8; ++i)
        fprintf(stderr, "%s%02X", i ? " " : "", (unsigned int)p[i]);
}

void sp_reply_diag_record(const char *stage, unsigned long long txn, int rc,
                          const void *tbuf, const void *outbuf, const void *inbuf,
                          int outcount, int calculated_length, const void *argbuf)
{
    unsigned long long seq = increment(&event_sequence);
    const unsigned char *tb = (const unsigned char *)tbuf;
    const unsigned char *ab = (const unsigned char *)argbuf;
    int header_length = -1;
    if (tb != NULL)
        header_length = (int)tb[0] + 256 * (int)tb[1];
    else if (ab != NULL)
        header_length = (int)ab[0] + 256 * (int)ab[1];

    fprintf(stderr,
            "[SP_REPLY_DIAG] seq=%llu txn=%llu stage=%s rc=%d "
            "Tbuf=%p Tbuf8=",
            seq, txn, stage, rc, tbuf);
    dump8(tb);
    fprintf(stderr,
            " tag=%u OutBuf=%p InBuf=%p OutCount=%d "
            "length_field=%d calculated_length=%d arg=%p arg8=",
            tb ? (unsigned int)tb[2] : 0u, outbuf, inbuf, outcount,
            header_length, calculated_length, argbuf);
    dump8(ab);
    fputc('\n', stderr);
    fflush(stderr);
}

void sp_reply_diag_put_count(const char *stage, unsigned long long txn,
                             unsigned int len, const void *len_address,
                             const void *tbuf, const void *outbuf, int outcount)
{
    unsigned long long seq = increment(&event_sequence);
    const unsigned char *tb = (const unsigned char *)tbuf;
    fprintf(stderr,
            "[SP_REPLY_DIAG] seq=%llu txn=%llu stage=%s len=%u len_hex=0x%08X "
            "len_address=%p Tbuf=%p Tbuf4=",
            seq, txn, stage, len, len, len_address, tbuf);
    if (tb == NULL)
        fputs("null", stderr);
    else
        fprintf(stderr, "%02X %02X %02X %02X", tb[0], tb[1], tb[2], tb[3]);
    fprintf(stderr, " OutBuf=%p OutCount=%d\n", outbuf, outcount);
    fflush(stderr);
}

void sp_reply_diag_link_read(const char *phase, const char *edge,
                             unsigned int packet_offset, unsigned int requested,
                             int returned, const void *buffer)
{
    const char *trace = getenv("T425_LINKIO_READ_TRACE");
    const char *capture = getenv("T425_LINKIO_READ_CAPTURE");
    static unsigned long long active_call = 0;
    unsigned long long seq, mono_ns = 0;
    unsigned int i, bytes = 0;
    const unsigned char *p = (const unsigned char *)buffer;
    if (!trace || strcmp(trace, "1") != 0)
        return;
    if (strcmp(edge, "BEGIN") == 0)
        active_call = increment(&read_call_sequence);
    seq = increment(&event_sequence);
    {
        struct timespec ts;
        if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
            mono_ns = (unsigned long long)ts.tv_sec * 1000000000ULL + (unsigned long long)ts.tv_nsec;
    }
    if (strcmp(edge, "END") == 0 && returned > 0 &&
        capture && strcmp(capture, "I_UNDERSTAND") == 0)
        bytes = (unsigned int)returned < requested ? (unsigned int)returned : requested;
    if (bytes > 8u) bytes = 8u;
    fprintf(stderr,
        "[LINKIO_DIAG] diag_seq=%llu event=LOPS_READ_CONTEXT pid=%ld call=%llu "
        "phase=%s edge=%s mono_ns=%llu packet_offset=%u requested=%u returned=%d "
        "buffer=%p capture=%s bytes=%u hex=",
        seq, (long)getpid(), active_call, phase, edge, mono_ns,
        packet_offset, requested, returned, buffer,
        (capture && strcmp(capture, "I_UNDERSTAND") == 0) ? "enabled" : "disabled", bytes);
    if (bytes == 0)
        fputs("-", stderr);
    else if (p != NULL)
        for (i = 0; i < bytes; ++i) fprintf(stderr, "%02X", (unsigned int)p[i]);
    fputc('\n', stderr);
    fflush(stderr);
}
