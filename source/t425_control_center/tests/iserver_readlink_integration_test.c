#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <libusb.h>
#include "../iserver/linkusb_linkio.h"
#include "../iserver/original/iserver/linkops/src/linkops.h"
#include "../iserver/original/iserver/linkops/src/opserror.h"

char lnkops_errorstring[1024];
static const unsigned char stream[8] = {0x01,0x00,0x02,0x00,0x00,0x00,0x00,0x00};
static unsigned int offset, calls;
void t425_linkio_install_libusb_read_hook(int (*hook)(unsigned char *, int, int, int *));
static int fragmented_bulk_read(unsigned char *buffer, int count, int timeout, int *transferred) {
    unsigned int n = (unsigned int)count < 2u ? (unsigned int)count : 2u;
    (void)timeout; *transferred = 0;
    ++calls;
    if (offset >= sizeof(stream)) return LIBUSB_ERROR_TIMEOUT;
    if (n > sizeof(stream) - offset) n = sizeof(stream) - offset;
    memcpy(buffer, stream + offset, n);
    offset += n;
    *transferred = (int)n;
    return 0;
}
int META_ReadLink(int id, char *buffer, unsigned int count, int timeout) {
    return ReadLink(id, buffer, count, timeout);
}
int META_TestError(int id) { (void)id; return 0; }
extern unsigned char LOPS_GetRequest(long int, unsigned char *, long int,
                                     unsigned char (*)(), enum ErrorModes);

int main(void) {
    unsigned char packet[64] = {0};
    unsigned char result;
    t425_linkio_install_libusb_read_hook(fragmented_bulk_read);
    result = LOPS_GetRequest(0, packet, 1000000L, NULL, ERRORignore);
    assert(result == STATUS_NOERROR);
    assert(offset == sizeof(stream));
    assert(calls >= 4); /* 1-byte initial call, then 2+2+2+1 for remaining header */
    assert(memcmp(packet, stream, sizeof(stream)) == 0);
    puts("LOPS_GetRequest_fragmented_header: PASS bytes=8 order=preserved");
    return 0;
}
