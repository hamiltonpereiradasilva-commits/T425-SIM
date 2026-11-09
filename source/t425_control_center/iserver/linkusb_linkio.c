#include "linkusb_linkio.h"
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <libusb.h>

#define USB_VID 0x1209
#define USB_PID 0x4250
#define USB_CFG 1
#define USB_INTF 0
#define EP_OUT 0x02
#define EP_IN 0x86
#define EP0_B4 0xB4
#define LINK_ID 0
#define LINKIO_ID 0

int METAmethod = 0;
int METAsearch_database = 1;
static libusb_context *ctx;
static libusb_device_handle *dev;
static int lock_fd = -1;
static int opened;
static t425_linkio_ops_t injected;
static int use_injected;
#ifdef T425_LINKIO_ENABLE_TEST_HOOK
static int (*test_bulk_read_hook)(unsigned char *, int, int, int *);
void t425_linkio_install_libusb_read_hook(int (*hook)(unsigned char *, int, int, int *)) {
    test_bulk_read_hook = hook;
}
#endif

#ifdef T425_LINKIO_WRITE_DIAG_TEST
static int (*test_b4_hook)(unsigned char out[56]);
static int (*test_bulk_out_hook)(unsigned char, unsigned char *, int, int *, int);
static unsigned long long (*test_now_ms_hook)(void);
void t425_linkio_install_write_diag_hooks(
    int (*b4_hook)(unsigned char out[56]),
    int (*bulk_hook)(unsigned char, unsigned char *, int, int *, int),
    unsigned long long (*clock_hook)(void)) {
    test_b4_hook = b4_hook;
    test_bulk_out_hook = bulk_hook;
    test_now_ms_hook = clock_hook;
}
void t425_linkio_test_set_open(int value) { opened = value; }
#endif

void t425_linkio_install_test_ops(const t425_linkio_ops_t *ops) {
    if (ops) { injected = *ops; use_injected = 1; }
    else { memset(&injected, 0, sizeof(injected)); use_injected = 0; }
}


static int acquire_lock(void) {
    const char *path = "/tmp/t425-sim-linkusb0.lock";
    lock_fd = open(path, O_CREAT | O_RDWR, 0600);
    if (lock_fd < 0) return -1;
    if (flock(lock_fd, LOCK_EX | LOCK_NB) != 0) {
        close(lock_fd); lock_fd = -1; return -1;
    }
    return 0;
}
static void release_lock(void) {
    if (lock_fd >= 0) { flock(lock_fd, LOCK_UN); close(lock_fd); lock_fd = -1; }
}
static int b4(unsigned char out[56]) {
    int n;
#ifdef T425_LINKIO_WRITE_DIAG_TEST
    if (test_b4_hook) return test_b4_hook(out);
#endif
    if (!dev) return -1;
    n = libusb_control_transfer(dev, 0xC0, EP0_B4, 0, LINK_ID, out, 56, 100);
    if (n != 56 || memcmp(out, "F271", 4) || out[4] != 1 || out[5] != LINK_ID) return -1;
    return 0;
}
static uint16_t le16(const unsigned char *p) { return (uint16_t)p[0] | ((uint16_t)p[1] << 8); }
static uint32_t le32(const unsigned char *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static int ack_delta(uint32_t before, uint32_t after) { return (uint32_t)(after-before) != 0; }
static unsigned long long now_ms(void) {
#ifdef T425_LINKIO_WRITE_DIAG_TEST
    if (test_now_ms_hook) return test_now_ms_hook();
#endif
    struct timeval tv; gettimeofday(&tv, NULL);
    return (unsigned long long)tv.tv_sec*1000ULL + (unsigned long long)tv.tv_usec/1000ULL;
}
static int bulk_out(unsigned char *data, int length, int *transferred, int timeout_ms) {
#ifdef T425_LINKIO_WRITE_DIAG_TEST
    if (test_bulk_out_hook) return test_bulk_out_hook(EP_OUT, data, length, transferred, timeout_ms);
#endif
    return libusb_bulk_transfer(dev, EP_OUT, data, length, transferred, timeout_ms);
}
static int endpoints_valid(void) {
    struct libusb_config_descriptor *cfg = NULL;
    libusb_device *d = libusb_get_device(dev);
    int i, j, found_out = 0, found_in = 0, ok = 0;
    if (libusb_get_active_config_descriptor(d, &cfg) != 0 || !cfg) return 0;
    for (i=0; i<cfg->bNumInterfaces; ++i) {
        const struct libusb_interface *intf = &cfg->interface[i];
        int a;
        for (a=0; a<intf->num_altsetting; ++a) {
            const struct libusb_interface_descriptor *alt = &intf->altsetting[a];
            if (alt->bInterfaceNumber != USB_INTF) continue;
            for (j=0; j<alt->bNumEndpoints; ++j) {
                const struct libusb_endpoint_descriptor *ep = &alt->endpoint[j];
                if (ep->bmAttributes == LIBUSB_TRANSFER_TYPE_BULK && ep->wMaxPacketSize == 64) {
                    if (ep->bEndpointAddress == EP_OUT) found_out = 1;
                    if (ep->bEndpointAddress == EP_IN) found_in = 1;
                }
            }
        }
    }
    ok = found_out && found_in;
    libusb_free_config_descriptor(cfg);
    return ok;
}
static unsigned int linkio_timeout_ms(int timeout) {
    if (timeout <= 0) return 0;
    if ((unsigned int)timeout > 600000u/100u) return 600000u;
    return (unsigned int)timeout * 100u; /* INMOS LinkIO timeout unit is 0.1 s. */
}

int OpenLink(char *name) {
    int rc, config = 0;
    if (use_injected) return injected.open ? injected.open(name) : -1;
    if (opened) return -1;
    if (name && strcmp(name, "linkusb0") != 0 && strcmp(name, "usb0") != 0) return -1;
    if (acquire_lock() != 0) return -1;
    rc = libusb_init(&ctx);
    if (rc < 0) goto fail;
    dev = libusb_open_device_with_vid_pid(ctx, USB_VID, USB_PID);
    if (!dev) goto fail;
    if (libusb_get_configuration(dev, &config) != 0 || config != USB_CFG || !endpoints_valid()) goto fail;
    if (libusb_claim_interface(dev, USB_INTF) != 0) goto fail;
    {
        unsigned char snap[56];
        if (b4(snap) != 0) goto fail_claim;
    }
    opened = 1;
    METAsearch_database = 0;
    METAmethod = LINKIO_ID;
    return 0;
fail_claim:
    libusb_release_interface(dev, USB_INTF);
fail:
    if (dev) { libusb_close(dev); dev = NULL; }
    if (ctx) { libusb_exit(ctx); ctx = NULL; }
    release_lock();
    return -1;
}

int CloseLink(int id) {
    int rc = 0;
    if (use_injected) return injected.close ? injected.close(id) : -1;
    if (!opened || id != LINKIO_ID) return -1;
    if (libusb_release_interface(dev, USB_INTF) != 0) rc = -1;
    libusb_close(dev); dev = NULL;
    libusb_exit(ctx); ctx = NULL;
    opened = 0;
    release_lock();
    return rc;
}

static int monotonic_ns(unsigned long long *value);
static volatile unsigned long long write_diag_sequence;
typedef struct {
    unsigned long long call_id;
    unsigned long long start_ns;
    unsigned long long end_ns;
    unsigned long long deadline_ms;
    unsigned int count;
    unsigned int done;
    unsigned int byte_index;
    unsigned int timeout_ms;
    uint32_t ack_before;
    uint32_t ack_after;
    unsigned int flags;
    unsigned int queue_depth;
    int libusb_rc;
    int transferred;
    int wait_b4_failures;
    const char *cause;
} write_diag_t;

static int write_diag_finish(write_diag_t *d, int result) {
    monotonic_ns(&d->end_ns);
    fprintf(stderr,
        "[LINKIO_WRITE_DIAG] call=%llu pid=%ld count=%u timeout_ms=%u done=%u byte_index=%u "
        "ack_rx_before=%u ack_rx_after=%u flags=0x%02x tx_queue_depth=%u "
        "libusb_rc=%d transferred=%d wait_b4_failures=%d deadline_ms=%llu "
        "start_mono_ns=%llu end_mono_ns=%llu elapsed_ns=%llu cause=%s result=%d\n",
        d->call_id, (long)getpid(), d->count, d->timeout_ms, d->done, d->byte_index,
        d->ack_before, d->ack_after, d->flags, d->queue_depth,
        d->libusb_rc, d->transferred, d->wait_b4_failures, d->deadline_ms,
        d->start_ns, d->end_ns, d->end_ns >= d->start_ns ? d->end_ns - d->start_ns : 0,
        d->cause, result);
    fflush(stderr);
    return result;
}

int WriteLink(int id, char *buffer, unsigned int count, int timeout) {
    unsigned int done = 0;
    if (use_injected) {
        for (; done < count; ++done) {
            int rc = injected.write_byte_wait_ack ? injected.write_byte_wait_ack((unsigned char)buffer[done], timeout) : -1;
            if (rc <= 0) return done ? (int)done : -1;
        }
        return (int)done;
    }
    {
        write_diag_t diag;
        memset(&diag, 0, sizeof(diag));
        diag.call_id = __sync_add_and_fetch(&write_diag_sequence, 1ULL);
        diag.count = count;
        diag.timeout_ms = linkio_timeout_ms(timeout);
        diag.ack_before = UINT32_MAX;
        diag.ack_after = UINT32_MAX;
        diag.flags = UINT_MAX;
        diag.queue_depth = UINT_MAX;
        diag.libusb_rc = INT_MIN;
        diag.transferred = INT_MIN;
        monotonic_ns(&diag.start_ns);
        if (!opened || id != LINKIO_ID || (!buffer && count) || timeout <= 0) {
            diag.cause = "INVALID_SESSION_OR_ARGUMENT";
            diag.done = 0;
            diag.byte_index = 0;
            return write_diag_finish(&diag, -1);
        }
    for (; done < count; ++done) {
        unsigned char before[56], after[56];
        int transferred = 0, rc;
        unsigned long long deadline;
        uint32_t ack_before;
        int preflight_rc;
        diag.byte_index = done;
        if (b4(before) != 0) {
            diag.cause = "B4_READ_FAILED";
            diag.done = done;
            return write_diag_finish(&diag, done ? (int)done : -1);
        }
        diag.flags = before[6];
        diag.queue_depth = le16(before+40);
        diag.ack_before = le32(before+20);
        diag.ack_after = diag.ack_before;
        preflight_rc = (before[6] & 2u) ? 1 : 0;
        if (preflight_rc) {
            diag.cause = "TX_WAITING_ACK";
            diag.done = done;
            return write_diag_finish(&diag, done ? (int)done : -1);
        }
        if (le16(before+40) != 0u) {
            diag.cause = "TX_QUEUE_BUSY";
            diag.done = done;
            return write_diag_finish(&diag, done ? (int)done : -1);
        }
        ack_before = le32(before+20);
        rc = bulk_out((unsigned char *)&buffer[done], 1, &transferred, (int)linkio_timeout_ms(timeout));
        diag.libusb_rc = rc;
        diag.transferred = transferred;
        if (rc != 0) {
            diag.cause = "LIBUSB_BULK_ERROR";
            diag.done = done;
            return write_diag_finish(&diag, done ? (int)done : -1);
        }
        if (transferred != 1) {
            diag.cause = "LIBUSB_PARTIAL_TRANSFER";
            diag.done = done;
            return write_diag_finish(&diag, done ? (int)done : -1);
        }
        deadline = now_ms() + linkio_timeout_ms(timeout);
        diag.deadline_ms = deadline;
        while (now_ms() < deadline) {
            if (b4(after) == 0) {
                diag.ack_after = le32(after+20);
                if (ack_delta(ack_before, diag.ack_after)) break;
            } else {
                ++diag.wait_b4_failures;
            }
            usleep(10000);
        }
        if (now_ms() >= deadline) {
            diag.cause = "ACK_TIMEOUT";
            diag.done = done;
            return write_diag_finish(&diag, done ? (int)done : -1);
        }
        /* for-loop increment advances after ACK-confirmed progress. */
    }
        diag.cause = "WRITE_COMPLETED";
        diag.done = count;
        diag.byte_index = count;
        return write_diag_finish(&diag, (int)count);
    }
}

static int monotonic_ns(unsigned long long *value) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return -1;
    *value = (unsigned long long)ts.tv_sec * 1000000000ULL + (unsigned long long)ts.tv_nsec;
    return 0;
}

static volatile unsigned long long read_sequence;
static unsigned long long next_read_sequence(void) {
    return __sync_add_and_fetch(&read_sequence, 1ULL);
}

static int pause_read_retry(unsigned long long deadline_ns) {
    unsigned long long now_ns, wake_ns, wait_ns;
    struct timespec req, rem;
    if (monotonic_ns(&now_ns) != 0) return -1;
    if (now_ns >= deadline_ns) return 0;
    wake_ns = now_ns + 1000000ULL;
    if (wake_ns > deadline_ns) wake_ns = deadline_ns;
    wait_ns = wake_ns - now_ns;
    req.tv_sec = (time_t)(wait_ns / 1000000000ULL);
    req.tv_nsec = (long)(wait_ns % 1000000000ULL);
    while (nanosleep(&req, &rem) != 0) {
        if (errno != EINTR) return -1;
        req = rem;
    }
    return 0;
}

static void log_read_result(unsigned long long sequence, unsigned long long started_ns,
                            unsigned long long finished_ns, unsigned int count,
                            int timeout, unsigned int timeout_ms, unsigned int done,
                            const char *reason, int capture, const char *buffer) {
    fprintf(stderr,
        "[LINKIO_DIAG] seq=%llu event=summary start_mono_ns=%llu end_mono_ns=%llu count=%u timeout=%d timeout_ms=%u accumulated=%u reason=%s pid=%ld session_open=%d device_handle=%d lock_held=%d\n",
        sequence, started_ns, finished_ns, count, timeout, timeout_ms, done, reason,
        (long)getpid(), opened, dev != NULL, lock_fd >= 0);
    if (capture && buffer && done) {
        unsigned int i, limit = done < 64u ? done : 64u;
        fprintf(stderr, "[LINKIO_DIAG] seq=%llu event=payload bytes=%u truncated=%s hex=",
                sequence, limit, done > limit ? "yes" : "no");
        for (i = 0; i < limit; ++i) fprintf(stderr, "%02X", (unsigned char)buffer[i]);
        fputc('\n', stderr);
    }
}

static int read_trace_enabled(void) {
    const char *value = getenv("T425_LINKIO_READ_TRACE");
    return value && strcmp(value, "1") == 0;
}

static int read_payload_capture_enabled(void) {
    const char *value = getenv("T425_LINKIO_READ_CAPTURE");
    return value && strcmp(value, "I_UNDERSTAND") == 0;
}

static int read_fragment(unsigned char *dst, int want, int timeout_ms, int *transferred) {
    int rc;
    *transferred = 0;
#ifdef T425_LINKIO_ENABLE_TEST_HOOK
    if (test_bulk_read_hook) return test_bulk_read_hook(dst, want, timeout_ms, transferred);
#endif
    if (use_injected) {
        int n;
        if (!injected.read_bytes) return LIBUSB_ERROR_IO;
        n = injected.read_bytes((char *)dst, (unsigned int)want, timeout_ms);
        if (n < 0) return LIBUSB_ERROR_IO;
        if (n == 0) return LIBUSB_ERROR_TIMEOUT;
        if (n > want) return LIBUSB_ERROR_OVERFLOW;
        *transferred = n;
        return 0;
    }
    rc = libusb_bulk_transfer(dev, EP_IN, dst, want, transferred, timeout_ms);
    return rc;
}

int ReadLink(int id, char *buffer, unsigned int count, int timeout) {
    unsigned int done = 0;
    unsigned int timeout_ms = 0;
    unsigned long long started_ns = 0, deadline_ns, sequence, rejected_ns = 0;
    int trace, capture, test_hook_active = 0;
#ifdef T425_LINKIO_ENABLE_TEST_HOOK
    test_hook_active = test_bulk_read_hook != NULL;
#endif
    sequence = next_read_sequence();
    trace = read_trace_enabled();
    capture = trace && read_payload_capture_enabled();
    if ((!use_injected && !test_hook_active && (!opened || id != LINKIO_ID)) ||
        ((use_injected || test_hook_active) && id != LINKIO_ID) ||
        (!buffer && count) || timeout <= 0 || count > (unsigned int)INT_MAX) {
        (void)monotonic_ns(&rejected_ns);
        if (trace) log_read_result(sequence, rejected_ns, rejected_ns, count, timeout, 0, 0, "INVALID_ARGUMENT", 0, NULL);
        return -1;
    }
    if (count == 0) {
        (void)monotonic_ns(&rejected_ns);
        if (trace) log_read_result(sequence, rejected_ns, rejected_ns, 0, timeout, 0, 0, "COUNT_COMPLETE", 0, NULL);
        return 0;
    }
    timeout_ms = linkio_timeout_ms(timeout);
    if (timeout_ms == 0 || monotonic_ns(&started_ns) != 0) {
        (void)monotonic_ns(&rejected_ns);
        if (trace) log_read_result(sequence, started_ns, rejected_ns, count, timeout, timeout_ms, 0, "CLOCK_OR_TIMEOUT_ERROR", 0, NULL);
        return -1;
    }
    deadline_ns = started_ns + (unsigned long long)timeout_ms * 1000000ULL;

    while (done < count) {
        unsigned long long call_start_ns, call_end_ns, now_ns = 0, remain_ns;
        unsigned int remaining = count - done;
        unsigned int call_timeout_ms;
        int transferred = 0, rc;
        if (monotonic_ns(&now_ns) != 0) {
            if (trace) log_read_result(sequence, started_ns, now_ns, count, timeout, timeout_ms, done, "CLOCK_ERROR", 0, NULL);
            return -1;
        }
        if (now_ns >= deadline_ns) {
            if (trace) log_read_result(sequence, started_ns, now_ns, count, timeout, timeout_ms, done, "GLOBAL_DEADLINE", capture, buffer);
            return (int)done;
        }
        remain_ns = deadline_ns - now_ns;
        call_timeout_ms = (unsigned int)((remain_ns + 999999ULL) / 1000000ULL);
        if (call_timeout_ms == 0) call_timeout_ms = 1;
        if (call_timeout_ms > timeout_ms) call_timeout_ms = timeout_ms;
        if (monotonic_ns(&call_start_ns) != 0) {
            if (trace) log_read_result(sequence, started_ns, now_ns, count, timeout, timeout_ms, done, "CLOCK_ERROR", 0, NULL);
            return -1;
        }
        rc = read_fragment((unsigned char *)buffer + done, (int)remaining,
                           (int)call_timeout_ms, &transferred);
        if (monotonic_ns(&call_end_ns) != 0) {
            if (trace) log_read_result(sequence, started_ns, call_start_ns, count, timeout, timeout_ms, done, "CLOCK_ERROR", 0, NULL);
            return -1;
        }
        if (transferred < 0 || (unsigned int)transferred > remaining) {
            if (trace) log_read_result(sequence, started_ns, call_end_ns, count, timeout, timeout_ms, done, "INVALID_TRANSFER_COUNT", 0, NULL);
            return -1;
        }
        if (rc == LIBUSB_ERROR_TIMEOUT) {
            done += (unsigned int)transferred;
            if (trace) fprintf(stderr,
                "[LINKIO_DIAG] seq=%llu event=fragment start_mono_ns=%llu end_mono_ns=%llu count=%u remaining=%u timeout=%d timeout_ms=%u libusb_rc=%d transferred=%d accumulated=%u result=TIMEOUT\n",
                sequence, call_start_ns, call_end_ns, count, remaining, timeout, call_timeout_ms, rc, transferred, done);
            if (done == count) {
                if (trace) log_read_result(sequence, started_ns, call_end_ns, count, timeout, timeout_ms, done, "COUNT_COMPLETE", capture, buffer);
                return (int)done;
            }
            if (call_end_ns >= deadline_ns) {
                if (trace) log_read_result(sequence, started_ns, call_end_ns, count, timeout, timeout_ms, done, "GLOBAL_DEADLINE", capture, buffer);
                return (int)done;
            }
            if (transferred == 0 && pause_read_retry(deadline_ns) != 0) {
                if (monotonic_ns(&now_ns) == 0 && trace) log_read_result(sequence, started_ns, now_ns, count, timeout, timeout_ms, done, "WAIT_ERROR", 0, NULL);
                return -1;
            }
            continue;
        }
        if (rc != 0) {
            if (trace) fprintf(stderr,
                "[LINKIO_DIAG] seq=%llu event=fragment start_mono_ns=%llu end_mono_ns=%llu count=%u remaining=%u timeout=%d timeout_ms=%u libusb_rc=%d transferred=%d accumulated=%u result=FATAL_ERROR\n",
                sequence, call_start_ns, call_end_ns, count, remaining, timeout, call_timeout_ms, rc, transferred, done);
            if (trace) log_read_result(sequence, started_ns, call_end_ns, count, timeout, timeout_ms, done, "FATAL_LIBUSB_ERROR", 0, NULL);
            return -1;
        }
        done += (unsigned int)transferred;
        if (trace) fprintf(stderr,
            "[LINKIO_DIAG] seq=%llu event=fragment start_mono_ns=%llu end_mono_ns=%llu count=%u remaining=%u timeout=%d timeout_ms=%u libusb_rc=%d transferred=%d accumulated=%u result=CONTINUE\n",
            sequence, call_start_ns, call_end_ns, count, remaining, timeout, call_timeout_ms, rc, transferred, done);
        if (transferred == 0 && pause_read_retry(deadline_ns) != 0) {
            if (monotonic_ns(&now_ns) == 0 && trace) log_read_result(sequence, started_ns, now_ns, count, timeout, timeout_ms, done, "WAIT_ERROR", 0, NULL);
            return -1;
        }
    }

    if (trace) {
        unsigned long long finished_ns = 0;
        (void)monotonic_ns(&finished_ns);
        log_read_result(sequence, started_ns, finished_ns, count, timeout, timeout_ms, done,
                        "COUNT_COMPLETE", capture, buffer);
    }
    return (int)done;
}
int TestError(int id) { (void)id; return -1; } /* No verified remote-error telemetry in B4. */
int TestRead(int id) {
    if (use_injected) return injected.test_read ? injected.test_read() : -1;
    if (!opened || id != LINKIO_ID) return -1;
    { unsigned char s[56]; if (b4(s) != 0) return -1; return le16(s+42) ? 1 : 0; }
}
int TestWrite(int id) {
    if (use_injected) return injected.test_write ? injected.test_write() : -1;
    if (!opened || id != LINKIO_ID) return -1;
    { unsigned char s[56]; if (b4(s) != 0) return -1; return (s[6] & 2u) == 0 && le32(s+48) != 0 && le16(s+40) == 0 ? 1 : 0; }
}
int ResetLink(int id) { (void)id; return -1; }
int AnalyseLink(int id) { (void)id; return -1; }
