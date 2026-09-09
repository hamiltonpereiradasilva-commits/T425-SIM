#include <assert.h>
#include <stdio.h>
#include "t425_stream.h"
#include "t425_reliable.h"
int main(void){
 t425_frame_t in={.src=0,.dst=1,.type=T425_DATA_TYPE,.seq=42,.len=5,.payload={9,8,7,6,5}},out;
 uint8_t raw[256];uint16_t n; t425_stream_parser_t p;
 assert(t425_frame_encode(&in,raw,sizeof(raw),&n));
 t425_stream_init(&p);
 bool got=false;for(uint16_t i=0;i<n;i++) if(t425_stream_push(&p,raw[i],&out))got=true;
 assert(got&&out.seq==42&&out.len==5&&out.payload[0]==9&&p.frames_ok==1);
 raw[10]^=0x55;t425_stream_init(&p);got=false;
 for(uint16_t i=0;i<n;i++) if(t425_stream_push(&p,raw[i],&out))got=true;
 assert(!got&&p.frames_bad==1);
 puts("BYTE STREAM PARSER + CRC: PASS");return 0;
}
