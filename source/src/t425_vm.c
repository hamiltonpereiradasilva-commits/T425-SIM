#include <string.h>
#include "t425_vm.h"

static uint32_t rd32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1]<<8) |
           ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24);
}
static void wr32(uint8_t *p,uint32_t v) {
    p[0]=(uint8_t)v; p[1]=(uint8_t)(v>>8);
    p[2]=(uint8_t)(v>>16); p[3]=(uint8_t)(v>>24);
}
static void push(t425_cpu_t *c,uint32_t x) {
    c->C=c->B; c->B=c->A; c->A=x;
}
static uint32_t pop(t425_cpu_t *c) {
    uint32_t x=c->A; c->A=c->B; c->B=c->C; return x;
}
static bool in_ram(uint32_t a,uint32_t n) {
    return a>=T425_INT_RAM_BASE && n<=T425_RAM_BYTES &&
           a-T425_INT_RAM_BASE+n<=T425_RAM_BYTES;
}
static uint32_t rw(t425_vm_t *vm,uint32_t a) {
    if(!in_ram(a,4)){vm->cpu.error=1;return 0;}
    return rd32(&vm->ram[a-T425_INT_RAM_BASE]);
}
static void ww(t425_vm_t *vm,uint32_t a,uint32_t v) {
    if(!in_ram(a,4)){vm->cpu.error=1;return;}
    wr32(&vm->ram[a-T425_INT_RAM_BASE],v);
}

/* Prefix semantics from T425 encoding:
   PFIX extends a nibble accumulator; NFIX complements it.
   The following instruction supplies the final nibble. */
static uint32_t operand_value(t425_vm_t *vm,uint8_t nibble) {
    uint32_t v = vm->cpu.prefix_valid ? ((vm->cpu.OPR<<4) | nibble) : nibble;
    vm->cpu.prefix_valid=0;
    vm->cpu.prefix_negative=0;
    vm->cpu.OPR=0;
    return v;
}
static void prefix_pfix(t425_vm_t *vm,uint8_t n) {
    vm->cpu.OPR=(vm->cpu.OPR<<4)|n;
    vm->cpu.prefix_valid=1;
}
static void prefix_nfix(t425_vm_t *vm,uint8_t n) {
    vm->cpu.OPR=~((vm->cpu.OPR<<4)|n);
    vm->cpu.prefix_valid=1;
    vm->cpu.prefix_negative=1;
}

static void operate(t425_vm_t *vm,uint32_t op) {
    t425_cpu_t *c=&vm->cpu; uint32_t x;
    switch(op) {
    case 0x00: x=c->A;c->A=c->B;c->B=x;break;             /* rev */
    case 0x01: {uint32_t a=c->A;if(in_ram(a,1))c->A=vm->ram[a-T425_INT_RAM_BASE];else c->error=1;}break;
    case 0x02: c->A=c->A-c->B; break;                     /* bsub */
    case 0x04: x=pop(c);c->A=x-c->A;break;                /* diff */
    case 0x05: x=pop(c);c->A=x+c->A;break;                /* add */
    case 0x09: x=pop(c);c->A=(x>c->A);break;              /* gt */
    case 0x0A: c->A=(c->A>>2);break;                     /* wsub: word index */
    case 0x0C: x=pop(c);c->A=x-c->A;break;                /* sub */
    case 0x0D: {                                          /* startp: phase-2 model */
        uint32_t pid=pop(c);
        if(pid<T425_MAX_PROCESSES){
            vm->proc[pid].state=PROC_READY;
            vm->proc[pid].pc=vm->proc[pid].pc?vm->proc[pid].pc:c->A;
        }
        break;
    }
    case 0x0F: c->A=~c->A; break;                         /* placeholder safety */
    case 0x12: c->error=1; break;                         /* resetch hook */
    case 0x18: c->A=vm->proc[0].pc;break;
    case 0x19: {                                          /* norm */
        uint32_t v=c->A,n=0;if(v==0){c->A=32;break;}
        while((v&0x80000000u)==0){v<<=1;n++;}
        c->A=n;break;
    }
    case 0x1A: x=pop(c);c->A=(c->A==0)?0:(uint32_t)((int32_t)x/(int32_t)c->A);break;
    case 0x1B: c->A=c->I;break;                           /* ldpi */
    case 0x1D: break;                                     /* xdble hook */
    case 0x1E: c->A=vm->current_priority;break;           /* ldpri */
    case 0x1F: x=pop(c);c->A=(c->A==0)?0:(uint32_t)((int32_t)x%(int32_t)c->A);break;
    case 0x20:                                             /* ret: return address in A */
        c->PC=c->A; c->A=c->B;c->B=c->C;c->C=0; break;
    case 0x21: break;                                     /* lend hook */
    case 0x22: c->A=vm->timer;break;                      /* ldtimer */
    case 0x29: c->error=0; c->A=0;break;                  /* testerr */
    case 0x2A: c->A=vm->analyse?1:0;break;
    case 0x2B: break;                                     /* tin hook */
    case 0x2C: x=pop(c);c->A=(c->A==0)?0:(uint32_t)((int32_t)x/(int32_t)c->A);break;
    case 0x2E: break;                                     /* dist hook */
    case 0x30: break;                                     /* diss */
    case 0x32: c->A=~c->A;break;                          /* not */
    case 0x33: x=pop(c);c->A=x^c->A;break;                /* xor */
    case 0x35: x=pop(c);c->A=x>>c->A;break;               /* lshr approximation */
    case 0x36: x=pop(c);c->A=x<<c->A;break;               /* lshl approximation */
    case 0x38: x=pop(c);c->A=x-c->A;break;                /* lsub */
    case 0x39: vm->proc[vm->current_pid].state=PROC_READY;break; /* runp hook */
    case 0x3A: break;                                     /* xword */
    case 0x3B: {uint32_t a=c->B;if(in_ram(a,1))vm->ram[a-T425_INT_RAM_BASE]=(uint8_t)c->A;else c->error=1;}break;
    case 0x3D: break; case 0x3E: break;
    case 0x40: x=pop(c);c->A=x>>c->A;break;               /* shr */
    case 0x41: x=pop(c);c->A=x<<c->A;break;               /* shl */
    case 0x42: c->A=(int32_t)c->A < 0 ? c->A : 0x7fffffff;break; /* mint model */
    case 0x46: x=pop(c);c->A=x&c->A;break;                /* and */
    case 0x4B: x=pop(c);c->A=x|c->A;break;                /* or */
    case 0x4E: break; case 0x4F: break;
    case 0x50: break; case 0x51: break;
    case 0x52: x=pop(c);c->A=x+c->A;break;                /* sum */
    case 0x53: x=pop(c);c->A=(uint32_t)((int32_t)x*(int32_t)c->A);break;
    case 0x54: vm->timer=0;break;                         /* sttimer */
    case 0x5A: push(c,c->A);break;                        /* dup */
    case 0x63: break; case 0x72: x=pop(c);c->A=x*c->A;break;
    case 0x73: break; case 0x74: break; case 0x75: break;
    case 0x76: {uint32_t v=c->A,n=0;while(v){n+=v&1;v>>=1;}c->A=n;}break;
    case 0x77: {uint32_t v=c->A,r=0;for(int i=0;i<32;i++){r=(r<<1)|(v&1);v>>=1;}c->A=r;}break;
    case 0x78: {uint32_t n=c->A&31,v=pop(c),r=0;for(uint32_t i=0;i<n;i++)r|=((v>>i)&1u)<<(n-1-i);c->A=r;}break;
    case 0x79: pop(c);break;                              /* pop */
    case 0x7A: case 0x7B: case 0x7C: case 0x7D: break;
    case 0x7E: c->A=T425_MEMSTART;break;                 /* ldmemstartval */
    default: c->error=1;c->halted=1;break;
    }
}
void t425_vm_reset(t425_vm_t *vm) {
    memset(vm,0,sizeof(*vm));
    vm->cpu.PC=T425_MEMSTART; vm->cpu.I=T425_MEMSTART; vm->cpu.W=T425_MEMSTART;
    vm->cpu.halted=1;
    for(unsigned i=0;i<T425_MAX_PROCESSES;i++){
        vm->proc[i].pid=i; vm->proc[i].state=(i==0)?PROC_READY:PROC_FREE;
        vm->proc[i].pc=T425_MEMSTART; vm->proc[i].w=T425_MEMSTART;
    }
    vm->current_pid=0; vm->current_priority=0;
}
bool t425_vm_load(t425_vm_t *vm,uint32_t addr,const uint8_t *data,uint32_t len){
    if(!data||!in_ram(addr,len))return false;
    memcpy(&vm->ram[addr-T425_INT_RAM_BASE],data,len);
    vm->cpu.PC=addr;vm->cpu.I=addr;vm->cpu.halted=0;return true;
}
bool t425_vm_read_mem(t425_vm_t *vm,uint32_t addr,uint8_t *out,uint32_t len){
    if(!out||!in_ram(addr,len)) return false;
    memcpy(out,&vm->ram[addr-T425_INT_RAM_BASE],len);
    return true;
}
bool t425_vm_write_mem(t425_vm_t *vm,uint32_t addr,const uint8_t *data,uint32_t len){
    if(!data||!in_ram(addr,len)) return false;
    memcpy(&vm->ram[addr-T425_INT_RAM_BASE],data,len);
    return true;
}
bool t425_vm_step(t425_vm_t *vm){
    t425_cpu_t*c=&vm->cpu;if(c->halted)return false;
    if(!in_ram(c->PC,1)){c->error=1;c->halted=1;return false;}
    uint8_t ins=vm->ram[c->PC++],fn=ins>>4,nib=ins&15;uint32_t op;
    switch(fn){
    case 0x0: op= c->prefix_valid?((c->OPR<<4)|nib):nib; c->prefix_valid=0;c->OPR=0;c->PC+=(int32_t)op;break;
    case 0x1: op=operand_value(vm,nib);push(c,c->W+op*4u);break;
    case 0x2: prefix_pfix(vm,nib);break;
    case 0x3: op=operand_value(vm,nib);c->A=rw(vm,c->A+op*4u);break;
    case 0x4: op=operand_value(vm,nib);push(c,op);break;
    case 0x5: op=operand_value(vm,nib);c->A+=op*4u;break;
    case 0x6: prefix_nfix(vm,nib);break;
    case 0x7: op=operand_value(vm,nib);push(c,rw(vm,c->W+op*4u));break;
    case 0x8: op=operand_value(vm,nib);c->A+=op;break;
    case 0x9: op=operand_value(vm,nib);push(c,c->PC);c->PC+=(int32_t)op;break;
    case 0xA: op=operand_value(vm,nib);if(c->A!=0){c->A=c->B;c->B=c->C;c->PC+=(int32_t)op;}else{c->A=c->B;c->B=c->C;}break;
    case 0xB: op=operand_value(vm,nib);c->W+=(int32_t)op*4;break;
    case 0xC: op=operand_value(vm,nib);c->A=(c->A==op);break;
    case 0xD: op=operand_value(vm,nib);ww(vm,c->W+op*4,pop(c));break;
    case 0xE: op=operand_value(vm,nib);ww(vm,c->A+op*4,c->B);break;
    case 0xF: op=operand_value(vm,nib);operate(vm,op);break;
    }
    vm->instructions++; return !c->halted;
}
void t425_vm_run_slice(t425_vm_t *vm,uint32_t budget){while(budget--&&!vm->cpu.halted)t425_vm_step(vm);}
void t425_vm_get_regs(const t425_vm_t *vm,uint32_t out[6]){
    out[0]=vm->cpu.A;out[1]=vm->cpu.B;out[2]=vm->cpu.C;out[3]=vm->cpu.W;out[4]=vm->cpu.I;out[5]=vm->cpu.PC;
}
void t425_vm_tick(t425_vm_t *vm){ if(vm->timer!=0xFFFFFFFFu) vm->timer++; }
void t425_vm_halt(t425_vm_t *vm){vm->cpu.halted=1;}
void t425_vm_run(t425_vm_t *vm){vm->cpu.halted=0;}
