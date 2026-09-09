#include "t425_frame.h"
#include "t425_crc16.h"
#include <string.h>
bool t425_frame_encode(const t425_frame_t*f,uint8_t*out,uint16_t cap,uint16_t*used){
 if(!f||!out||!used||f->len>T425_FRAME_MAX_PAYLOAD||cap<(uint16_t)(7+f->len))return false;
 out[0]=T425_FRAME_SOF;out[1]=f->src;out[2]=f->dst;out[3]=f->type;out[4]=f->seq;out[5]=f->len;
 if(f->len)memcpy(&out[6],f->payload,f->len);
 uint16_t c=t425_crc16(out,6+f->len);out[6+f->len]=(uint8_t)c;out[7+f->len]=(uint8_t)(c>>8);
 *used=(uint16_t)(8+f->len);return true;
}
bool t425_frame_decode(const uint8_t*in,uint16_t n,t425_frame_t*f){
 if(!in||!f||n<8||in[0]!=T425_FRAME_SOF)return false;
 if(in[5]>T425_FRAME_MAX_PAYLOAD||n!=(uint16_t)(8+in[5]))return false;
 uint16_t c=t425_crc16(in,6+in[5]), got=(uint16_t)in[6+in[5]]|((uint16_t)in[7+in[5]]<<8);
 if(c!=got)return false;
 f->src=in[1];f->dst=in[2];f->type=in[3];f->seq=in[4];f->len=in[5];
 if(f->len) memcpy(f->payload,&in[6],f->len);
 f->crc=got;
 return true;
}
