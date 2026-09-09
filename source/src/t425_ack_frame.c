#include "t425_ack_frame.h"
#include <string.h>
bool t425_make_ack(uint8_t src,uint8_t dst,uint8_t seq,t425_frame_t*f){
 if(!f)return false;memset(f,0,sizeof(*f));
 f->src=src;f->dst=dst;f->type=T425_ACK_FRAME_TYPE;f->seq=seq;f->len=0;return true;
}
