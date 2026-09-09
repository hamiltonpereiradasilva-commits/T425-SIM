#include "t425_occam.h"
bool t425_occam_validate(const t425_occam_program_t*p){
    if(!p||!p->code||p->length==0||p->length>T425_OCCAM_MAX_OPS)return false;
    for(uint16_t i=0;i<p->length;i++){
        switch(p->code[i].op){
            case OSEQ: case OIN: case OOUT: case OALT:
            case OTIMER_AFTER: case OEND: break;
            default:return false;
        }
    }
    return true;
}
