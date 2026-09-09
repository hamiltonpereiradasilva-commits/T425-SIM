#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "t425_crc16.h"
int main(void){
    const uint8_t s[]="123456789";
    assert(t425_crc16(s,9)==0x4B37);
    puts("CRC16/Modbus test vector: PASS");
    return 0;
}
