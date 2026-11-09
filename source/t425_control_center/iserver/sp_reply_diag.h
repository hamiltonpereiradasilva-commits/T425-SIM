#ifndef T425_SP_REPLY_DIAG_H
#define T425_SP_REPLY_DIAG_H

/* Experimental observation only; no SP buffers or return values are changed. */
unsigned long long sp_reply_diag_begin(int is_sp_puts);
unsigned long long sp_reply_diag_current(void);
int sp_reply_diag_is_puts(void);
void sp_reply_diag_record(const char *stage, unsigned long long txn, int rc,
                          const void *tbuf, const void *outbuf, const void *inbuf,
                          int outcount, int calculated_length, const void *argbuf);
void sp_reply_diag_put_count(const char *stage, unsigned long long txn,
                             unsigned int len, const void *len_address,
                             const void *tbuf, const void *outbuf, int outcount);
void sp_reply_diag_link_read(const char *phase, const char *edge,
                             unsigned int packet_offset, unsigned int requested,
                             int returned, const void *buffer);

#endif
