#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "t425_vm_host_adapter.h"
#include "t425_core_phy_ipc.h"
#include "t425_channel_dma_ack_adapter_09.h"
#include "t425_channel_dma_tx_coordinator_45.h"
#include "f248_tx_transport.h"
static t425_core_phy_ipc_t ipc;
static t425_core_phy_ipc_service_t service;
static t425_channel_dma_ack_adapter_09_t ack;
static t425_channel_dma_tx_coordinator_45_t coordinator;
static t425_core_phy_tx_ack_event_fn ack_sink;
static void *ack_sink_context;
static t425_core_t *bound_core;
static int release_pending_release;
static uint32_t release_pending_channel, release_pending_request, release_pending_generation;
static uint32_t release_pending_buffer, release_pending_length;
static uint8_t release_pending_slot, release_pending_endpoint, release_pending_link;
static t425_channel_admission_entry_21_t *entry_for_channel(
    t425_core_t *core, uint32_t channel)
{
    if (!core) return 0;
    for (unsigned i=0; i<T425_CHANNEL_ADMISSION_21_CHANNELS; ++i)
        if (core->channel_admission.channel[i].architectural_channel == channel)
            return &core->channel_admission.channel[i];
    return 0;
}
#define CAP 65536u
static uint8_t bytes[CAP]; static size_t head,tail,used;
static t425_vm_exec_t *exec_bound;
static int (*progress_one)(void);
static uint64_t tx_n,ack_n,rx_n,round_n;
#define tx_bytes tx_n
#define ack_events ack_n
static int tx_frame(void *ctx,uint32_t ep,uint16_t frame){(void)ctx;if(ep||used==CAP)return -1;bytes[tail]=(uint8_t)frame;tail=(tail+1u)%CAP;++used;++tx_n;if(ack_sink){++ack_n;ack_sink(ack_sink_context,ep);}return 0;}
static bool rx_avail(void *ctx,uint32_t ep){(void)ctx;(void)ep;return false;}
static int rx_get(void *ctx,uint32_t ep,uint32_t *frame){(void)ctx;(void)ep;(void)frame;return -1;}
static int set_ack(void *ctx,t425_core_phy_tx_ack_event_fn fn,void *c){(void)ctx;ack_sink=fn;ack_sink_context=c;return 0;}
bool f248_tx_init(t425_core_t *core,t425_vm_exec_t *exec){t425_core_phy_ops_t ops={0};if(!core||!exec)return false;head=tail=used=0;tx_n=ack_n=rx_n=round_n=0;if(!t425_core_phy_ipc_init(&ipc)||!t425_channel_dma_ack_adapter_09_init(&ack,&ipc,&core->channel_dma_shift))return false;for(uint8_t ep=0;ep<4;ep++)if(!t425_channel_dma_ack_adapter_09_bind(&ack,ep,ep))return false;if(!t425_channel_dma_tx_coordinator_45_init(&coordinator,core,&ack,&ipc))return false;ops.tx_frame=tx_frame;ops.rx_available=rx_avail;ops.rx_get=rx_get;ops.set_tx_ack_sink=set_ack;if(!t425_core_phy_ipc_service_init(&service,&ipc,&ops))return false;bound_core=core;exec_bound=exec;return true;}
static void consider_active_tx(t425_core_t *core, uint8_t i) {
    t425_channel_admission_entry_21_t *e=&core->channel_admission.channel[i];
    t425_channel_admission_request_21_t snap={0}; uint8_t ep=e->endpoint;
    if(e->state!=T425_CHANNEL_ADMISSION_21_ACTIVE || e->direction!=T425_CHANNEL_DMA_TX || ep>=4u || coordinator.session[ep].valid) return;
    if(!t425_core_channel_admission_get_active(core,e->architectural_channel,e->request_id,e->operation_generation,e->endpoint,e->physical_link,&snap)) return;
    if(!coordinator.session[ep].lease.valid && !t425_core_phy_ipc_tx_lease_request(&ipc,ep,&coordinator.session[ep].lease)) return;
    (void)t425_core_phy_ipc_service_poll(&service);
    if(t425_core_phy_ipc_tx_lease_poll_acquire(&coordinator.session[ep].lease)!=1) return;
    (void)t425_channel_dma_tx_coordinator_45_bind_active(&coordinator,e->architectural_channel,e->request_id,e->operation_generation,e->endpoint,e->physical_link,snap.lane,&coordinator.session[ep].lease);
}
static int rearm_released_shift(t425_core_t *core,
    uint32_t channel, t425_channel_dma_direction_t direction,
    uint8_t endpoint, uint8_t slot, uint8_t physical_link,
    uint32_t request_id, uint32_t generation,
    uint32_t session_buffer, uint32_t session_length)
{
    t425_channel_admission_entry_21_t *entry =
        entry_for_channel(core,channel);
    t425_channel_dma_05_tx_state_t *shift =
        slot<T425_CHANNEL_DMA_SLOT_COUNT ? &core->channel_dma_shift.tx[slot] : 0;
    t425_channel_dma_lane_t *lane =
        slot<T425_CHANNEL_DMA_SLOT_COUNT ? &core->t425_channel_dma.tx[slot] : 0;
    int pending_match =
        release_pending_release &&
        release_pending_channel==channel &&
        release_pending_slot==slot &&
        release_pending_endpoint==endpoint &&
        release_pending_request==request_id &&
        release_pending_generation==generation &&
        release_pending_buffer==session_buffer &&
        release_pending_length==session_length &&
        release_pending_link==physical_link;
    if (!release_pending_release) {
        release_pending_release=1;
        release_pending_channel=channel;
        release_pending_slot=slot;
        release_pending_endpoint=endpoint;
        release_pending_request=request_id;
        release_pending_generation=generation;
        release_pending_buffer=session_buffer;
        release_pending_length=session_length;
        release_pending_link=physical_link;
        pending_match=1;
    }
    int session_owner_valid =
        endpoint<T425_CORE_PHY_MAX_ENDPOINTS &&
        coordinator.session[endpoint].retired_request_id==request_id &&
        coordinator.session[endpoint].retired_operation_generation==generation &&
        coordinator.session[endpoint].valid==0u &&
        coordinator.session[endpoint].releasing==0u &&
        direction==T425_CHANNEL_DMA_TX &&
        slot<T425_CHANNEL_DMA_SLOT_COUNT &&
        entry!=0 && entry->architectural_channel==channel &&
        entry->direction==T425_CHANNEL_DMA_TX &&
        entry->endpoint==endpoint && entry->physical_link==physical_link &&
        entry->lane==slot && entry->request_id==request_id &&
        entry->operation_generation==generation &&
        entry->buffer_address==session_buffer &&
        entry->length==session_length &&
        entry->initial_count_reg==(uint32_t)(0u-session_length) &&
        entry->state==T425_CHANNEL_ADMISSION_21_ACTIVE &&
        entry->owner_is_f05!=0u &&
        shift!=0 && lane!=0 &&
        shift->configured &&
        shift->buffer_start_address==session_buffer &&
        shift->initial_count_reg==(uint32_t)(0u-session_length);
    int drained = session_owner_valid &&
        !shift->active &&
        shift->stage==T425_CHANNEL_DMA_05_TX_IDLE &&
        shift->ticket==0u &&
        lane->shift_valid_bits==0u &&
        lane->count_reg==0u &&
        t425_channel_dma_shift_05_tx_complete(
            &core->channel_dma_shift,slot);

    if (!pending_match || !session_owner_valid || !drained) {
        {}
        return 0;
    }

    uint32_t count_pre=lane->count_reg, ptr_pre=lane->ptr_reg;
    uint32_t dbuff_pre=lane->dbuff_reg;
    t425_channel_dma_05_status_t rc=t425_channel_dma_shift_05_try_abort(
        &core->channel_dma_shift,T425_CHANNEL_DMA_TX,slot);
    int post_ok=rc==T425_CHANNEL_DMA_05_OK &&
        !shift->configured && !shift->active &&
        shift->stage==T425_CHANNEL_DMA_05_TX_IDLE &&
        shift->buffer_start_address==0u && shift->initial_count_reg==0u &&
        shift->ticket==0u && shift->next_request_id==0u &&
        lane->count_reg==count_pre && lane->ptr_reg==ptr_pre &&
        lane->dbuff_reg==dbuff_pre && lane->shift_valid_bits==0u;

    {}
    return post_ok;
}

void f248_tx_poll(t425_core_t *core) {
 if(!core || core!=bound_core) return;
 (void)t425_core_phy_ipc_service_poll(&service);
 for(uint8_t i=0;i<T425_CHANNEL_ADMISSION_21_CHANNELS;++i) consider_active_tx(core,i);
 for(uint8_t ep=0;ep<4u;++ep) if(coordinator.session[ep].valid) {
  (void)t425_channel_dma_tx_coordinator_45_submit_one(&coordinator,ep);
  (void)t425_core_phy_ipc_service_poll(&service);
  (void)t425_channel_dma_tx_coordinator_45_poll_ack(&coordinator,ep);
  if(t425_channel_dma_tx_coordinator_45_complete(&coordinator,ep))
   (void)t425_channel_dma_tx_coordinator_45_release_request(&coordinator,ep);
  (void)t425_core_phy_ipc_service_poll(&service);
  t425_channel_dma_tx_coord_45_session_t *live=&coordinator.session[ep];
uint32_t done_channel=live->session.architectural_channel;
uint32_t done_request=live->session.request_id;
uint32_t done_generation=live->session.operation_generation;
uint32_t done_buffer=live->session.buffer_address;
uint32_t done_length=live->session.length;
uint8_t done_link=live->session.physical_link;
uint8_t done_slot=live->dma_slot;
t425_channel_dma_direction_t done_direction=live->session.direction;
int release_candidate=live->valid && live->releasing &&
    done_direction==T425_CHANNEL_DMA_TX && done_channel!=0u &&
    done_request!=0u && done_generation!=0u &&
    !(live->retired_request_id==done_request &&
      live->retired_operation_generation==done_generation);
if(release_candidate)
    {}
int pr=t425_channel_dma_tx_coordinator_45_poll_release(&coordinator,ep);
if(pr && release_candidate) {
    t425_channel_admission_entry_21_t *entry=
        entry_for_channel(core,done_channel);
    {}
    if(coordinator.session[ep].retired_request_id!=done_request ||
       coordinator.session[ep].retired_operation_generation!=done_generation ||
       entry==0 || entry->direction!=T425_CHANNEL_DMA_TX ||
       entry->request_id!=done_request ||
       entry->operation_generation!=done_generation ||
       entry->state!=T425_CHANNEL_ADMISSION_21_ACTIVE ||
       entry->owner_is_f05==0u) {
        {}
    } else if(rearm_released_shift(core,done_channel,done_direction,
               ep,done_slot,done_link,done_request,done_generation,
               done_buffer,done_length)) {
        {}
        {}
        int complete_rc=t425_core_channel_admission_report_complete(
            core,done_channel,done_generation);
        {}
        entry=entry_for_channel(core,done_channel);
        if(complete_rc && entry &&
           entry->state==T425_CHANNEL_ADMISSION_21_COMPLETED &&
           entry->operation_generation==done_generation &&
           entry->request_id==done_request &&
           entry->owner_is_f05==0u) {
            {}
            int retire_rc=t425_core_channel_admission_retire(
                core,done_channel,done_generation);
            {}
            entry=entry_for_channel(core,done_channel);
            if(!retire_rc || !entry ||
               entry->state==T425_CHANNEL_ADMISSION_21_ACTIVE ||
               entry->state==T425_CHANNEL_ADMISSION_21_COMPLETED ||
               entry->operation_generation!=0u ||
               entry->request_id!=0u)
                {}
            else
                release_pending_release=0;
        } else {
            {}
        }
    } else {
        {}
    }
} else if(pr) {
    {}
}}}
int f248_link_read(uint8_t ep,uint8_t *out,size_t count,int timeout){size_t got=0;uint64_t rounds=0,max=timeout>0?(uint64_t)(unsigned)timeout*10000u:100000u;if(ep||(!out&&count))return -1;if(max>10000000u)max=10000000u;while(got<count&&rounds<max){while(got<count&&used){out[got++]=bytes[head];head=(head+1u)%CAP;--used;++rx_n;}if(got==count||rounds>=max)break;if(progress_one ? progress_one()!=0 : (!exec_bound||t425_vm_exec_round(exec_bound)!=T425_VM_EXEC_OK))break;++rounds;++round_n;}return (int)got;}
void f248_tx_shutdown(void){if(!ipc.initialized)return;{}for(uint32_t i=0;i<T425_CORE_PHY_MAX_ENDPOINTS;i++){queue_free(&ipc.core0_to_core1[i]);queue_free(&ipc.core1_to_core0[i]);}bound_core=NULL;exec_bound=NULL;}

void f248_tx_set_progress(int (*fn)(void)){progress_one=fn;}
size_t f248_tx_queued(void){return used;}
uint64_t f248_tx_pushed(void){return tx_n;}
uint64_t f248_tx_popped(void){return rx_n;}

uint64_t f248_tx_acked(void){return ack_n;}
