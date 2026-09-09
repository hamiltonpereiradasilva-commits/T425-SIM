#include "t425_reliable.h"
#include <string.h>
void t425_reliable_init(t425_reliable_t*r){memset(r,0,sizeof(*r));r->last_rx_seq=0xFF;}
bool t425_reliable_prepare(t425_reliable_t*r,t425_frame_t*f,uint8_t src,uint8_t dst,const uint8_t*d,uint8_t len){
 if(!r||!f||len>T425_FRAME_MAX_PAYLOAD)return false;
 memset(f,0,sizeof(*f));f->src=src;f->dst=dst;f->type=T425_DATA_TYPE;f->seq=r->next_seq++;f->len=len;
 if(len&&d) memcpy(f->payload,d,len);
 return true;
}
bool t425_reliable_is_ack(const t425_frame_t*f,uint8_t seq){return f&&f->type==T425_ACK_TYPE&&f->seq==seq;}
bool t425_reliable_is_duplicate(t425_reliable_t*r,const t425_frame_t*f){
 if(!r||!f)return true;
 if(f->seq==r->last_rx_seq){r->duplicates++;return true;}
 r->last_rx_seq=f->seq;return false;
}
void t425_reliable_note_tx(t425_reliable_t*r){if(r)r->tx_count++;}
void t425_reliable_note_ack(t425_reliable_t*r){if(r)r->ack_count++;}
void t425_reliable_note_retry(t425_reliable_t*r){if(r){r->retry_count++;r->retries++;}}
void t425_reliable_note_timeout(t425_reliable_t*r){if(r)r->timeouts++;}
void t425_reliable_note_rx(t425_reliable_t*r){if(r)r->rx_count++;}
void t425_reliable_note_crc_error(t425_reliable_t*r){if(r)r->crc_errors++;}
