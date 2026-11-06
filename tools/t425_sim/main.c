#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t425_vm.h"
#include "t425_vm_transport.h"
#include "t425_vm_exec.h"
#include "t425_vm_host_adapter.h"
#include "t425_vm_linkio.h"
#include "t425_vm_linkio_facade.h"
#include "t425_vm_linkio_compat.h"
#include "t425_vm_memory_manager.h"
#include "t425_channel_compat_rx_adapter_07.h"
#include "t425_sim_link_service.h"
#include "f248_tx_transport.h"
extern int t425_inmos_iserver_main(int,char **);
static t425_core_t sim_core;
static t425_vm_exec_t sim_exec;
static t425_vm_host_adapter_t sim_host;
static t425_vm_memory_manager_t memory_manager;
static t425_channel_compat_input_t compat_input;
static t425_channel_compat_rx_adapter_07_t compat_rx;
static uint8_t *backing;
static int masked_write(void *context,uint32_t address,uint32_t value,uint32_t mask) {
    t425_vm_memory_manager_t *m=context;
    uint8_t bytes[4]={(uint8_t)value,(uint8_t)(value>>8),(uint8_t)(value>>16),(uint8_t)(value>>24)};
    if(!m || (address&3u) || !mask || mask>15u) return -1;
    for(size_t i=0;i<m->region_count;++i) {
        t425_vm_memory_region_t *r=&m->regions[i];
        if(!(r->flags&T425_VM_MEMORY_WRITE) || address<r->t425_base) continue;
        uint32_t off=address-r->t425_base;
        if(off>r->size || 4u>r->size-off) continue;
        for(unsigned b=0;b<4;++b) if(mask&(1u<<b)) r->backing[off+b]=bytes[b];
        return 0;
    }
    return -1;
}
int main(int argc,char **argv) {
    t425_vm_t vm={0}; t425_vm_transport_t transport={0};
    t425_vm08_linkio_t *primitive=NULL; t425_vm08f_linkio_t *facade=NULL;
    t425_vm08g_context_t *context=NULL; t425_vm08g_binding_t binding;
    static const uint8_t lanes[4]={0,1,2,3};
    const char *profile=getenv("T425_ISERVER_SPID_PROFILE");
    if(profile && *profile && strcmp(profile,"compat-v14")) { fputs("Unsupported T425-SIM profile\n",stderr); return 64; }
    if(argc<2) { fprintf(stderr,"usage: %s [INMOS IServer options]\n",argv[0]); return 2; }
    assert(t425_vm_init(&vm)); memset(&sim_core,0,sizeof sim_core); t425_core_reset(&sim_core);
    assert(t425_vm_memory_manager_init(&memory_manager)==0);
    backing=calloc(0x3ff000u,1u); assert(backing);
    assert(t425_vm_memory_add_region(&memory_manager,0x80001000u,0x3ff000u,backing,T425_VM_MEMORY_READ|T425_VM_MEMORY_WRITE)==0);
    t425_memory_set_external_backend(&sim_core.memory,t425_vm_memory_manager_external_read,t425_vm_memory_manager_external_write,&memory_manager);
    t425_memory_set_external_masked_write_backend(&sim_core.memory,masked_write);
    sim_core.cpu.IptrReg=T425_INTERNAL_RAM_BASE;
    assert(t425_memory_write_byte(&sim_core.memory,T425_INTERNAL_RAM_BASE,0u)==0);
    assert(t425_vm_add_emulated_node(&vm,0,&sim_core));
    assert(t425_vm_transport_init(&transport,&vm));
    assert(t425_vm_exec_init(&sim_exec,&vm,&transport,NULL,NULL));
    assert(t425_vm_host_adapter_init(&sim_host,&vm,&transport,(t425_vm_endpoint_t){0,0}));
    t425_sim_link_attach(&sim_core,&sim_exec,&sim_host);
    assert(t425_vm08_open(&primitive,&sim_host));
    assert(t425_vm08f_open(&facade,primitive,t425_sim_virtual_now_ms,NULL,t425_sim_progress,NULL,100u));
    binding.name="T425SIM0"; binding.facade=facade;
    assert(t425_vm08g_context_create(&context,&binding,1u,0u));
    assert(t425_vm08g_install_context(context));
    for(uint32_t i=0;i<8u;++i) assert(t425_core_channel_admission_configure_route(&sim_core,0x80000000u+4u*i,(uint8_t)(i%4u),(uint8_t)(i%4u)));
    t425_channel_compat_input_init(&compat_input);
    assert(t425_channel_compat_rx_adapter_07_init(&compat_rx,sim_core.phy,&compat_input,&sim_core.channel_dma_memory));
    assert(t425_core_channel_compat_profile_08_enable(&sim_core,&compat_rx,lanes));
    f248_tx_set_progress(t425_sim_progress_one);
    assert(f248_tx_init(&sim_core,&sim_exec));
    assert(t425_vm_exec_rounds(&sim_exec,6u)==T425_VM_EXEC_OK);
    int status=t425_inmos_iserver_main(argc,argv);
    fprintf(stderr,"T425SIM_FINAL cpu_error=%u iserver_status=%d cycles=%llu instructions=%llu tx=%llu ack=%llu read=%llu queued=%zu\n",
        (unsigned)sim_core.cpu.error,status,(unsigned long long)sim_core.cycles,(unsigned long long)sim_core.instructions,
        (unsigned long long)f248_tx_pushed(),(unsigned long long)f248_tx_acked(),(unsigned long long)f248_tx_popped(),f248_tx_queued());
    f248_tx_shutdown();t425_vm08g_uninstall_context();t425_vm08g_context_destroy(context);
    t425_vm08f_close(facade);t425_vm08_close(primitive);t425_vm_host_adapter_destroy(&sim_host);
    t425_vm_exec_destroy(&sim_exec);t425_vm_transport_destroy(&transport);t425_vm_destroy(&vm);
    t425_memory_set_external_backend(&sim_core.memory,NULL,NULL,NULL);free(backing);
    return sim_core.cpu.error ? 1 : status;
}
