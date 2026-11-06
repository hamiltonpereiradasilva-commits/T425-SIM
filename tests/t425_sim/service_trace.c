/* Optional regression observer. No handler or response is implemented here. */
#include <stdio.h>
#include <stdlib.h>
#include "t425_sim_link_service.h"
#include "f248_tx_transport.h"
extern unsigned char __real_OPS_GetRequest(long int,unsigned char *,long int,unsigned char (*)(void));
extern unsigned char __real_OPS_BootWrite(long int,unsigned char *,unsigned long);
extern int __real_Serve(void);
extern void __real_SpCommand(void);
unsigned test_requests;
static int command_pending;
static void observe_reply(uint64_t seq,unsigned char result,const unsigned char *frame,int consumed) {
    (void)frame;
    fprintf(stderr,"T425SIM_REPLY seq=%llu rc=%u consumed=%d\n",(unsigned long long)seq,result,consumed);
    if(command_pending) {
        command_pending=0;
        if(getenv("T425_SIM_SMOKE_ON_COMMAND") && result==0 && consumed && !t425_sim_link_state()->core->cpu.error) {
            fprintf(stderr,"T425SIM_SMOKE_PASS cpu_error=0 tx=%llu ack=%llu read=%llu queued=%zu\n",(unsigned long long)f248_tx_pushed(),(unsigned long long)f248_tx_acked(),(unsigned long long)f248_tx_popped(),f248_tx_queued());
            exit(0);
        }
    }
}
__attribute__((constructor)) static void install_observer(void) { t425_sim_link_set_reply_observer(observe_reply); }
int __wrap_Serve(void) { fputs("T425SIM_SERVE_ENTER\n",stderr); return __real_Serve(); }
unsigned char __wrap_OPS_GetRequest(long int link,unsigned char *buffer,long int timeout,unsigned char (*heartbeat)(void)) {
    unsigned char result=__real_OPS_GetRequest(link,buffer,timeout,heartbeat);
    ++test_requests;
    fprintf(stderr,"T425SIM_REQUEST seq=%u rc=%u tag=%u\n",test_requests,result,result==0 ? buffer[2] : 0);
    return result;
}
unsigned char __wrap_OPS_BootWrite(long int link,unsigned char *buffer,unsigned long count) {
    unsigned char result=__real_OPS_BootWrite(link,buffer,count);
    fprintf(stderr,"T425SIM_BOOT count=%lu status=%u\n",count,result);return result;
}
void __wrap_SpCommand(void) { __real_SpCommand();command_pending=1;fputs("T425SIM_ORIGINAL_SPCOMMAND\n",stderr); }
