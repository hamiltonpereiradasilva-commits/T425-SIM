#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <libusb.h>
#include "../iserver/linkusb_linkio.h"

enum { MODE_FULL, MODE_1_6, MODE_2_5, MODE_BYTE, MODE_TIMEOUT_EMPTY,
       MODE_TIMEOUT_PROGRESS, MODE_TIMEOUT_EARLY_PROGRESS, MODE_TIMEOUT_EARLY_EMPTY,
       MODE_FATAL, MODE_DEADLINE };
static const unsigned char bytes[96] = {0x10,0x21,0x32,0x43,0x54,0x65,0x76,0x87};
static unsigned int offset;
static int mode, calls, lengths[128], timeouts[128];

void t425_linkio_install_libusb_read_hook(int (*hook)(unsigned char *, int, int, int *));

static unsigned long long monotonic_us(void) {
    struct timespec ts;
    assert(clock_gettime(CLOCK_MONOTONIC, &ts) == 0);
    return (unsigned long long)ts.tv_sec * 1000000ULL + (unsigned long long)ts.tv_nsec / 1000ULL;
}

static void reset_case(int next_mode) {
    mode = next_mode; offset = 0; calls = 0;
    memset(lengths, 0, sizeof(lengths)); memset(timeouts, 0, sizeof(timeouts));
}
static int fake_libusb_bulk_in(unsigned char *dst, int length, int timeout_ms, int *transferred) {
    unsigned int n;
    assert(calls < 128 && length > 0 && timeout_ms > 0);
    lengths[calls] = length; timeouts[calls] = timeout_ms; ++calls;
    *transferred = 0;
    if (mode == MODE_TIMEOUT_EMPTY) return LIBUSB_ERROR_TIMEOUT;
    if (mode == MODE_TIMEOUT_PROGRESS && calls == 2) {
        usleep((useconds_t)timeout_ms * 1000u);
        dst[0] = bytes[offset++]; *transferred = 1;
        return LIBUSB_ERROR_TIMEOUT;
    }
    if (mode == MODE_TIMEOUT_EARLY_PROGRESS && calls == 1) {
        dst[0] = bytes[offset++]; dst[1] = bytes[offset++]; *transferred = 2;
        return LIBUSB_ERROR_TIMEOUT;
    }
    if (mode == MODE_TIMEOUT_EARLY_EMPTY && calls == 1) return LIBUSB_ERROR_TIMEOUT;
    if (mode == MODE_FATAL && calls == 2) return LIBUSB_ERROR_NO_DEVICE;
    if (mode == MODE_DEADLINE && timeout_ms <= 60) {
        usleep((useconds_t)timeout_ms * 1000u);
        return LIBUSB_ERROR_TIMEOUT;
    }
    switch (mode) {
      case MODE_1_6: n = calls == 1 ? 1u : 6u; break;
      case MODE_2_5: n = calls == 1 ? 2u : 5u; break;
      case MODE_BYTE: case MODE_DEADLINE: n = 1u; break;
      case MODE_TIMEOUT_PROGRESS: n = 2u; break;
      case MODE_TIMEOUT_EARLY_PROGRESS: n = (unsigned int)length; break;
      case MODE_TIMEOUT_EARLY_EMPTY: n = (unsigned int)length; break;
      case MODE_FATAL: n = 2u; break;
      default: n = (unsigned int)length; break;
    }
    if (n > (unsigned int)length) n = (unsigned int)length;
    if (n > sizeof(bytes) - offset) n = sizeof(bytes) - offset;
    if (mode == MODE_DEADLINE) usleep(60000);
    memcpy(dst, bytes + offset, n); offset += n; *transferred = (int)n;
    return 0;
}
static void exact(int test_mode, unsigned int count, int fragments, const char *name) {
    unsigned char out[8] = {0};
    int i;
    reset_case(test_mode);
    assert(ReadLink(0, (char *)out, count, 100) == (int)count);
    assert(memcmp(out, bytes, count) == 0);
    assert(offset == count && calls == fragments);
    if (test_mode == MODE_1_6) assert(lengths[0] == 7 && lengths[1] == 6);
    if (test_mode == MODE_2_5) assert(lengths[0] == 7 && lengths[1] == 5);
    if (test_mode == MODE_BYTE)
        for (i = 0; i < calls; ++i) assert(lengths[i] == (int)(count - (unsigned int)i));
    printf("%s: PASS bytes=%u fragments=%d order=preserved\n", name, count, calls);
}

int main(void) {
    unsigned char out[8] = {0};
    unsigned char large[96] = {0};
    t425_linkio_install_libusb_read_hook(fake_libusb_bulk_in);
    exact(MODE_FULL, 8, 1, "full_transfer");
    exact(MODE_1_6, 7, 2, "fragment_1_plus_6");
    exact(MODE_2_5, 7, 2, "fragment_2_plus_5");
    exact(MODE_BYTE, 8, 8, "one_byte_per_transfer");

    reset_case(MODE_TIMEOUT_EMPTY);
    {
        unsigned long long start, end; int result;
        start = monotonic_us();
        result = ReadLink(0, (char *)out, 8, 1);
        end = monotonic_us();
        assert(result == 0 && calls > 1 && end-start < 500000ULL);
        printf("repeated_early_timeout_without_data: PASS requested_ms=100 effective_us=%llu transfers=%d accumulated=0 result=TIMEOUT\n", end-start, calls);
    }

    reset_case(MODE_TIMEOUT_PROGRESS);
    {
        unsigned long long start = monotonic_us(), end; int result;
        result = ReadLink(0, (char *)out, 8, 1);
        end = monotonic_us();
        assert(result == 3);
        printf("timeout_after_partial_progress: PASS requested_ms=100 effective_us=%llu transfers=%d accumulated=%d result=PARTIAL\n", end-start, calls, result);
    }
    assert(offset == 3 && memcmp(out, bytes, 3) == 0);
    puts("timeout_partial_prefix_integrity: PASS exact_prefix=YES");

    reset_case(MODE_TIMEOUT_EARLY_PROGRESS);
    {
        unsigned long long start = monotonic_us(), end; int result;
        result = ReadLink(0, (char *)out, 8, 10);
        end = monotonic_us();
        assert(result == 8);
        printf("intermediate_timeout_with_progress: PASS requested_ms=1000 effective_us=%llu transfers=%d accumulated=%d result=COMPLETE\n", end-start, calls, result);
    }
    assert(offset == 8 && calls == 2 && memcmp(out, bytes, 8) == 0);
    puts("intermediate_timeout_with_progress_global_time_remaining: PASS resumed=YES");

    reset_case(MODE_TIMEOUT_EARLY_EMPTY);
    {
        unsigned long long start = monotonic_us(), end; int result;
        result = ReadLink(0, (char *)out, 8, 10);
        end = monotonic_us();
        assert(result == 8);
        printf("intermediate_timeout_without_progress: PASS requested_ms=1000 effective_us=%llu transfers=%d accumulated=%d result=COMPLETE\n", end-start, calls, result);
    }
    assert(offset == 8 && calls == 2 && memcmp(out, bytes, 8) == 0);
    puts("intermediate_timeout_without_progress_global_time_remaining: PASS bounded_retry=YES");

    reset_case(MODE_FATAL);
    assert(ReadLink(0, (char *)out, 8, 1) == -1);
    assert(offset == 2 && memcmp(out, bytes, 2) == 0);
    puts("fatal_disconnect_after_progress: PASS returned=-1");

    reset_case(MODE_DEADLINE);
    {
        unsigned long long start, end; int result;
        start = monotonic_us();
        result = ReadLink(0, (char *)out, 8, 3);
        end = monotonic_us();
        assert(result > 0 && result < 8 && end-start < 1000000ULL && offset == (unsigned int)result);
        assert(calls == result || calls == result + 1);
        { int i; for (i=1; i<calls; ++i) assert(timeouts[i-1] >= timeouts[i]); }
        assert(memcmp(out, bytes, (size_t)result) == 0);
        printf("absolute_global_deadline: PASS requested_ms=300 effective_us=%llu transfers=%d accumulated=%d result=PARTIAL timeouts_nonincreasing=YES\n", end-start, calls, result);
    }

    reset_case(MODE_FULL);
    assert(ReadLink(0, (char *)out, 4, 100) == 4 && offset == 4);
    assert(ReadLink(0, (char *)out+4, 4, 100) == 4 && offset == 8);
    assert(lengths[0] == 4 && lengths[1] == 4);
    assert(memcmp(out, bytes, 8) == 0);
    puts("no_prefetch_no_loss_between_calls: PASS");

    reset_case(MODE_FULL);
    assert(ReadLink(0, (char *)out, 0, 100) == 0 && calls == 0);
    assert(ReadLink(0, (char *)out, 8, 0) == -1);
    assert(ReadLink(0, (char *)out, (unsigned int)INT_MAX + 1u, 1) == -1);
    puts("zero_count_and_length_bounds: PASS");

    reset_case(MODE_FULL);
    assert(ReadLink(0, (char *)large, 80, 100) == 80);
    assert(offset == 80 && memcmp(large, bytes, 80) == 0);
    puts("optional_payload_capture_limit_fixture: PASS bytes=80 expected_capture_limit=64");
    return 0;
}
