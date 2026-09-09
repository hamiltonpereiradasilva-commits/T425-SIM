#include "t425_stream.h"
#include <string.h>

void t425_stream_init(t425_stream_parser_t*p){
    memset(p,0,sizeof(*p));
    p->state=T425_STREAM_WAIT_SOF;
}

bool t425_stream_push(t425_stream_parser_t*p,uint8_t b,t425_frame_t*out){
    if(!p||!out)return false;
    p->bytes_seen++;
    switch(p->state){
    case T425_STREAM_WAIT_SOF:
        if(b==T425_FRAME_SOF){p->frame.len=0;p->header_pos=0;p->state=T425_STREAM_HEADER;}
        break;
    case T425_STREAM_HEADER:
        p->header[p->header_pos++]=b;
        if(p->header_pos==5){
            p->frame.src=p->header[0];p->frame.dst=p->header[1];
            p->frame.type=p->header[2];p->frame.seq=p->header[3];
            p->frame.len=p->header[4];
            if(p->frame.len>T425_FRAME_MAX_PAYLOAD){p->frames_bad++;p->state=T425_STREAM_WAIT_SOF;}
            else if(p->frame.len==0){p->crc_pos=0;p->state=T425_STREAM_CRC;}
            else {p->payload_pos=0;p->state=T425_STREAM_PAYLOAD;}
        }
        break;
    case T425_STREAM_PAYLOAD:
        p->frame.payload[p->payload_pos++]=b;
        if(p->payload_pos==p->frame.len){p->crc_pos=0;p->state=T425_STREAM_CRC;}
        break;
    case T425_STREAM_CRC:
        p->crc_bytes[p->crc_pos++]=b;
        if(p->crc_pos==2){
            uint8_t raw[8+T425_FRAME_MAX_PAYLOAD];
            uint16_t n=(uint16_t)(8+p->frame.len);
            raw[0]=T425_FRAME_SOF;raw[1]=p->frame.src;raw[2]=p->frame.dst;
            raw[3]=p->frame.type;raw[4]=p->frame.seq;raw[5]=p->frame.len;
            if(p->frame.len)memcpy(&raw[6],p->frame.payload,p->frame.len);
            raw[6+p->frame.len]=p->crc_bytes[0];raw[7+p->frame.len]=p->crc_bytes[1];
            bool ok=t425_frame_decode(raw,n,out);
            if(ok)p->frames_ok++;else p->frames_bad++;
            p->state=T425_STREAM_WAIT_SOF;
            return ok;
        }
        break;
    default:
        p->state=T425_STREAM_WAIT_SOF;
        break;
    }
    return false;
}
