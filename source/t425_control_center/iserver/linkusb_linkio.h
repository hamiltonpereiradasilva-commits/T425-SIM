#ifndef T425_LINKUSB_LINKIO_H
#define T425_LINKUSB_LINKIO_H
#include <stdint.h>

typedef struct {
    int (*open)(const char *resource);
    int (*close)(int id);
    int (*write_byte_wait_ack)(unsigned char byte, int timeout_ms);
    int (*read_bytes)(char *buffer, unsigned int count, int timeout_ms);
    int (*test_read)(void);
    int (*test_write)(void);
} t425_linkio_ops_t;

/* Standard INMOS LinkIO entry points used by the original LOPS backend. */
int OpenLink(char *name);
int CloseLink(int link_id);
int ReadLink(int link_id, char *buffer, unsigned int count, int timeout);
int WriteLink(int link_id, char *buffer, unsigned int count, int timeout);
int ResetLink(int link_id);
int AnalyseLink(int link_id);
int TestError(int link_id);
int TestRead(int link_id);
int TestWrite(int link_id);

/* Adapter seam used by hardware-free contract tests. */
void t425_linkio_install_test_ops(const t425_linkio_ops_t *ops);
extern int METAmethod;
extern int METAsearch_database;
#endif
