#include "t425_link_bench.h"
#include "t425_crc16.h"
#include <string.h>

bool t425_frame_build(t425_frame_t *f,uint8_t src,uint8_t dst,
                      uint8_t seq,const uint8_t *payload,uint8_t len){
    if(!f || len>32u)return false;
    memset(f,0,sizeof(*f));
    f->sync=T425_FRAME_SYNC;
    f->version=T425_FRAME_VERSION;
    f->src=src; f->dst=dst; f->seq=seq; f->len=len;
    if(len && payload) memcpy(f->payload,payload,len);
    f->crc16=t425_crc16_ccitt((const uint8_t*)f,
                               (uint16_t)(6u+len));
    return true;
}

bool t425_frame_check(const t425_frame_t *f){
    if(!f || f->sync!=T425_FRAME_SYNC ||
       f->version!=T425_FRAME_VERSION || f->len>32u)return false;
    return f->crc16==t425_crc16_ccitt((const uint8_t*)f,
                                      (uint16_t)(6u+f->len));
}
