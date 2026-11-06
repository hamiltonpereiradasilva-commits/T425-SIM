#ifndef F248_TX_TRANSPORT_H
#define F248_TX_TRANSPORT_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "t425_core.h"
#include "t425_vm_exec.h"
bool f248_tx_init(t425_core_t *, t425_vm_exec_t *);
void f248_tx_poll(t425_core_t *);
void f248_tx_shutdown(void);
uint64_t f248_tx_acked(void);
int f248_link_read(uint8_t, uint8_t *, size_t, int);
void f248_tx_set_progress(int (*fn)(void));
size_t f248_tx_queued(void);
uint64_t f248_tx_pushed(void);
uint64_t f248_tx_popped(void);
#endif
