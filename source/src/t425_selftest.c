#include "t425_selftest.h"
#include "t425_vm.h"
bool t425_selftest(void){
    t425_vm_t v; uint8_t p1[]={0x23,0x45},p2[]={0x61,0x41},p3[]={0x4A,0x54,0xF5};
    t425_vm_reset(&v); if(!t425_vm_load(&v,T425_MEMSTART,p1,2))return false;t425_vm_run_slice(&v,2);if(v.cpu.A!=0x35)return false;
    t425_vm_reset(&v); if(!t425_vm_load(&v,T425_MEMSTART,p2,2))return false;t425_vm_run_slice(&v,2);if(v.cpu.A!=0xFFFFFFE1u)return false;
    t425_vm_reset(&v); if(!t425_vm_load(&v,T425_MEMSTART,p3,3))return false;t425_vm_run_slice(&v,3);if(v.cpu.A!=30u)return false;
    return true;
}
