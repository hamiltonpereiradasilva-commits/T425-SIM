#include <stdio.h>
#include <string.h>
#include "t425_link_test.h"

int main(void){
    t425_test_frame_t in={.type=T425_TEST_PING,.seq=7,.len=4,.payload={0x54,0x50,0x49,0x4E}};
    uint8_t raw[64]; t425_test_frame_t out;
    uint16_t n=t425_test_encode(&in,raw,sizeof(raw));
    if(!n)return 1;
    t425_test_parser_reset();
    bool got=false;
    /* Noise before a valid frame must be ignored. */
    t425_test_decode_byte(0x00,&out);
    for(uint16_t i=0;i<n;i++)if(t425_test_decode_byte(raw[i],&out))got=true;
    if(!got || out.type!=T425_TEST_PING || out.seq!=7 || out.len!=4 ||
       memcmp(out.payload,in.payload,4)!=0)return 2;
    raw[n-1]^=0x01;
    t425_test_parser_reset(); got=false;
    for(uint16_t i=0;i<n;i++)if(t425_test_decode_byte(raw[i],&out))got=true;
    if(got)return 3;
    puts("T425 PHYSICAL LINK HARNESS: PASS");
    return 0;
}
