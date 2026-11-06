#include "t425_sim_link_service.h"
#include "f248_tx_transport.h"
static t425_sim_link_state_t link_state;
static t425_sim_reply_observer_fn reply_observer;
uint64_t service_reply_count;
extern bool __real_t425_core_channel_dma_service(t425_core_t *);
extern int __real_WriteLink(int, char *, unsigned int, int);
extern unsigned char __real_OPS_SendReply(long int, unsigned char *);
extern void __real_SpId(void);
extern unsigned char Tbuf[];
void t425_sim_link_attach(t425_core_t *core,t425_vm_exec_t *exec,t425_vm_host_adapter_t *host) {
    link_state=(t425_sim_link_state_t){core,exec,host,0}; service_reply_count=0;
}
const t425_sim_link_state_t *t425_sim_link_state(void) { return &link_state; }
void t425_sim_link_set_reply_observer(t425_sim_reply_observer_fn observer) { reply_observer=observer; }
int t425_sim_progress_one(void) {
    if(!link_state.exec || t425_vm_exec_round(link_state.exec)!=T425_VM_EXEC_OK) return -1;
    ++link_state.clock_ms; return 0;
}
uint64_t t425_sim_virtual_now_ms(void *unused) { (void)unused; return link_state.clock_ms; }
t425_vm08f_progress_result_t t425_sim_progress(void *unused) {
    (void)unused; return t425_sim_progress_one()==0 ? T425_VM08F_PROGRESS_OK : T425_VM08F_PROGRESS_ERROR;
}
bool __wrap_t425_core_channel_dma_service(t425_core_t *core) {
    bool result=__real_t425_core_channel_dma_service(core);
    f248_tx_poll(core); return result;
}
int __wrap_ReadLink(int link,char *buffer,unsigned int count,int timeout) {
    (void)link; return f248_link_read(0u,(uint8_t *)buffer,count,timeout);
}
int __wrap_WriteLink(int link,char *buffer,unsigned int count,int timeout) {
    unsigned int total=0;
    while(total<count) {
        unsigned int remain=count-total;
        int n=__real_WriteLink(link,buffer+total,remain,timeout);
        if(n<=0 || (unsigned)n>remain) break;
        total+=(unsigned)n;
    }
    return (int)total;
}
unsigned char __wrap_OPS_SendReply(long int link,unsigned char *frame) {
    unsigned char result=__real_OPS_SendReply(link,frame);
    ++service_reply_count;
    if(result==0u) {
        unsigned rounds=0;
        while(link_state.host && link_state.host->user_link.host_to_core_pending && rounds<1000000u) {
            if(t425_sim_progress_one()!=0) break;
            ++rounds;
        }
    }
    if(reply_observer) reply_observer(service_reply_count,result,frame,
        link_state.host && !link_state.host->user_link.host_to_core_pending);
    return result;
}
/* The approved compat-v14 profile adjusts only the original SpId reply. */
void __wrap_SpId(void) {
    __real_SpId();
    if(Tbuf[0]==6u && Tbuf[1]==0u && Tbuf[2]==0u && Tbuf[3]==15u && Tbuf[4]==0u && Tbuf[5]==0u && Tbuf[6]==0u) {
        Tbuf[0]=5u; Tbuf[1]=0u; Tbuf[2]=0u; Tbuf[3]=14u;
        Tbuf[4]=1u; Tbuf[5]=1u; Tbuf[6]=1u;
    }
}
