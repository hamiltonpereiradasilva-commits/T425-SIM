#include "t425_forward.h"

bool t425_forward_build(t425_forward_hdr_t*h,uint8_t src,uint8_t dst,
                        uint8_t final_dst,uint8_t ttl,uint8_t payload_len){
    if(!h || ttl==0u || ttl>T425_FWD_MAX_HOPS)return false;
    h->version=T425_FWD_VERSION;
    h->src=src; h->dst=dst; h->final_dst=final_dst;
    h->hop_count=0u; h->ttl=ttl; h->payload_len=payload_len;
    return true;
}

bool t425_forward_forwardable(const t425_forward_hdr_t*h){
    return h && h->version==T425_FWD_VERSION && h->ttl>0u &&
           h->hop_count<T425_FWD_MAX_HOPS;
}

void t425_forward_advance(t425_forward_hdr_t*h,uint8_t next_hop){
    if(!h || h->ttl==0u)return;
    h->dst=next_hop;
    h->ttl--;
    h->hop_count++;
}
