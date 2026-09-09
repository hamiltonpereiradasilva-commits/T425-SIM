#include "t425_link.h"
#include <string.h>

void t425_link_stack_init(t425_link_stack_t*s){
    memset(s,0,sizeof(*s));
    t425_reliable_init(&s->reliable);
}

bool t425_link_stack_receive(t425_link_stack_t*s,const uint8_t*raw,uint16_t n,t425_frame_t*out){
    if(!s||!raw||!out)return false;
    if(!t425_frame_decode(raw,n,out)){
        s->frames_bad++;
        t425_reliable_note_crc_error(&s->reliable);
        return false;
    }
    s->frames_ok++;
    t425_reliable_note_rx(&s->reliable);
    return true;
}

bool t425_link_stack_prepare(t425_link_stack_t*s,uint8_t src,uint8_t dst,
                             const uint8_t*payload,uint8_t len,uint16_t*wire_len){
    t425_frame_t f;
    if(!s||!wire_len)return false;
    if(!t425_reliable_prepare(&s->reliable,&f,src,dst,payload,len))return false;
    if(!t425_frame_encode(&f,s->tx_buf,sizeof(s->tx_buf),wire_len))return false;
    t425_reliable_note_tx(&s->reliable);
    return true;
}
