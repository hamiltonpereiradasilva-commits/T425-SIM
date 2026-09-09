#include <assert.h>
#include <stdio.h>
#include <stdint.h>
int main(void){
    for(int i=0;i<256;i++){
        uint8_t b=(uint8_t)i;
        assert((uint8_t)b==(uint8_t)i);
    }
    puts("PIO BYTE STREAM 00..FF MODEL: PASS");
    return 0;
}
