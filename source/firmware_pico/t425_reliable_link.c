#include "t425_reliable_link.h"
#include "t425_crc16.h"
#include <string.h>

static uint16_t frame_len(const t425_frame_t*f){ return (uint16_t)(T425_OVERHEAD+f->len); }

bool t425_frame_build(t425_frame_t*f,uint8_t type,uint8_t src,uint8_t dst,
                      uint8_t seq,const uint8_t*p,uint8_t len){
    if(!f||len>T425_MAX_PAYLOAD)return false;
    memset(f,0,sizeof(*f));
    f->sync=T425_SYNC; f->version=T425_VERSION; f->type=type;
    f->src=src; f->dst=dst; f->seq=seq; f->len=len;
    if(len&&p)memcpy(f->payload,p,len);
    f->crc16=t425_crc16_ccitt((const uint8_t*)f,(uint16_t)(T425_HEADER_LEN+len));
    return true;
}
bool t425_frame_check(const t425_frame_t*f){
    if(!f||f->sync!=T425_SYNC||f->version!=T425_VERSION||f->len>T425_MAX_PAYLOAD)return false;
    return f->crc16==t425_crc16_ccitt((const uint8_t*)f,(uint16_t)(T425_HEADER_LEN+f->len));
}

void t425_parser_init(t425_parser_t*p){
    memset(p,0,sizeof(*p));
}

bool t425_parser_push(t425_parser_t*p,uint8_t b,t425_frame_t*out){
    if(!p||!out)return false;

    if(p->pos==0){
        if(b!=T425_SYNC){ p->sync_losses++; return false; }
        p->buf[p->pos++]=b;
        return false;
    }

    p->buf[p->pos++]=b;

    if(p->pos==T425_HEADER_LEN){
        uint8_t version=p->buf[1];
        uint8_t type=p->buf[2];
        uint8_t len=p->buf[6];

        if(version!=T425_VERSION ||
           (type!=T425_TYPE_DATA && type!=T425_TYPE_ACK) ||
           len>T425_MAX_PAYLOAD){
            p->format_errors++;
            p->pos=0;
            p->expected=0;
            return false;
        }
        p->expected=(uint16_t)(T425_OVERHEAD+len);
    }

    if(p->expected && p->pos==p->expected){
        memcpy(out,p->buf,p->expected);
        p->pos=0;
        p->expected=0;

        if(!t425_frame_check(out)){
            p->crc_errors++;
            return false;
        }

        p->frames_ok++;
        return true;
    }

    if(p->pos>=sizeof(p->buf)){
        p->format_errors++;
        p->pos=0;
        p->expected=0;
    }
    return false;
}

void t425_frame_corrupt_crc(t425_frame_t*f){
    if(f) f->crc16 ^= 0x0001u;
}

void t425_rl_init(t425_rl_t*r,uint32_t timeout_ms,uint8_t max_retries){
    memset(r,0,sizeof(*r)); r->timeout_ms=timeout_ms; r->max_retries=max_retries;
}
bool t425_rl_accept_ack(t425_rl_t*r,uint8_t seq){
    if(!r||!r->waiting_ack||r->waiting_seq!=seq)return false;
    r->waiting_ack=false;r->acked++;return true;
}

bool t425_rl_accept_data(t425_rl_t*r,uint8_t seq,bool*duplicate){
    if(!r||!duplicate)return false;
    *duplicate=false;
    if(!r->have_rx_seq){
        r->have_rx_seq=true;
        r->last_rx_seq=seq;
        return true;
    }
    uint8_t expected=(uint8_t)(r->last_rx_seq+1u);
    if(seq==r->last_rx_seq){
        *duplicate=true;
        r->duplicate_data++;
        return false;
    }
    if(seq!=expected){
        r->out_of_order++;
        /* Accept the new sequence so a later valid stream can recover. */
        r->last_rx_seq=seq;
        return false;
    }
    r->last_rx_seq=seq;
    return true;
}

void t425_rl_start_tx(t425_rl_t*r,uint8_t seq,uint32_t now_ms){
    if(!r)return;
    r->pending_seq=seq;
    r->pending_valid=true;
    r->waiting_seq=seq;
    r->waiting_ack=true;
    r->retries=0;
    r->deadline_ms=now_ms+r->timeout_ms;
    r->tx_frames++;
}

bool t425_rl_timeout(t425_rl_t*r,uint32_t now_ms){
    if(!r||!r->waiting_ack)return false;
    return (int32_t)(now_ms-r->deadline_ms)>=0;
}

void t425_rl_note_retransmit(t425_rl_t*r,uint32_t now_ms){
    if(!r)return;
    r->retransmits++;
    r->retries++;
    r->deadline_ms=now_ms+r->timeout_ms;
}

void t425_rl_note_timeout(t425_rl_t*r){
    if(!r)return;
    r->timeouts++;
    r->waiting_ack=false;
    r->pending_valid=false;
    r->retries=0;
}
