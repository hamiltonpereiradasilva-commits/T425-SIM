#include "t425_link_test.h"
#include "t425_crc16.h"
#include <string.h>

static uint8_t buf[5+T425_TEST_MAX_PAYLOAD+2];
static uint16_t pos=0, expected=0;

uint16_t t425_test_encode(const t425_test_frame_t*f,uint8_t*out,uint16_t cap){
    if(!f||!out||f->len>T425_TEST_MAX_PAYLOAD)return 0;
    uint16_t n=(uint16_t)(5+f->len+2);
    if(cap<n)return 0;
    out[0]=T425_TEST_MAGIC0; out[1]=T425_TEST_MAGIC1;
    out[2]=f->type; out[3]=f->seq; out[4]=f->len;
    if(f->len) memcpy(&out[5],f->payload,f->len);
    uint16_t crc=t425_crc16_ccitt(out,5+f->len);
    out[5+f->len]=(uint8_t)(crc>>8);
    out[6+f->len]=(uint8_t)crc;
    return n;
}
void t425_test_parser_reset(void){pos=0;expected=0;}
bool t425_test_decode_byte(uint8_t b,t425_test_frame_t*out){
    if(!out)return false;
    if(pos==0 && b!=T425_TEST_MAGIC0)return false;
    if(pos==1 && b!=T425_TEST_MAGIC1){pos=0;return false;}
    if(pos>=sizeof(buf)){pos=0;expected=0;return false;}
    buf[pos++]=b;
    if(pos==5){
        if(buf[4]>T425_TEST_MAX_PAYLOAD){pos=0;expected=0;return false;}
        expected=(uint16_t)(7+buf[4]);
    }
    if(expected && pos==expected){
        uint16_t got=(uint16_t)((buf[expected-2]<<8)|buf[expected-1]);
        uint16_t calc=t425_crc16_ccitt(buf,expected-2);
        bool ok=(got==calc);
        if(ok){
            out->type=buf[2];out->seq=buf[3];out->len=buf[4];
            if(out->len)memcpy(out->payload,&buf[5],out->len);
        }
        pos=0;expected=0;return ok;
    }
    return false;
}
