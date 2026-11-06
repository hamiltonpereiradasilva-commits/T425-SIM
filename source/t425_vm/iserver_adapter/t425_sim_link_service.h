#ifndef T425_SIM_LINK_SERVICE_H
#define T425_SIM_LINK_SERVICE_H
#include <stdint.h>
#include "t425_core.h"
#include "t425_vm_exec.h"
#include "t425_vm_host_adapter.h"
#include "t425_vm_linkio_facade.h"
typedef struct {
    t425_core_t *core;
    t425_vm_exec_t *exec;
    t425_vm_host_adapter_t *host;
    uint64_t clock_ms;
} t425_sim_link_state_t;
typedef void (*t425_sim_reply_observer_fn)(uint64_t sequence, unsigned char result, const unsigned char *frame, int consumed);
void t425_sim_link_attach(t425_core_t *, t425_vm_exec_t *, t425_vm_host_adapter_t *);
const t425_sim_link_state_t *t425_sim_link_state(void);
void t425_sim_link_set_reply_observer(t425_sim_reply_observer_fn);
int t425_sim_progress_one(void);
uint64_t t425_sim_virtual_now_ms(void *);
t425_vm08f_progress_result_t t425_sim_progress(void *);
#endif
