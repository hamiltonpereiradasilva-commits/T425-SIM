#include <stdint.h>
uint16_t t425_crc16(const uint8_t *p, uint16_t n){
    uint16_t crc=0xFFFF;
    for(uint16_t i=0;i<n;i++){
        crc^=p[i];
        for(uint8_t b=0;b<8;b++)
            crc=(crc&1u)?(uint16_t)((crc>>1)^0xA001u):(uint16_t)(crc>>1);
    }
    return crc;
}
