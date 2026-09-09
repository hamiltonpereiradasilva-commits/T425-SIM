#include "t425_occam_parser.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
bool t425_occam_parse_demo(const char*s,t425_occam_instr_t*out,size_t cap,size_t*n){
    size_t k=0; const char*p=s;
    while(*p){
        while(*p && isspace((unsigned char)*p))p++;
        if(!*p)break;
        if(!strncmp(p,"OUT",3)){p+=3;while(*p&&isspace((unsigned char)*p))p++;if(*p!='(')return false;p++;
            int ch=atoi(p);while(*p&&*p!=',')p++;if(*p!=',')return false;p++;int val=atoi(p);
            while(*p&&*p!=')') p++;
            if(*p!=')'||k>=cap) return false;
            p++;
            out[k++]=(t425_occam_instr_t){OOUT,(uint16_t)ch,0,(uint32_t)val};
        } else if(!strncmp(p,"IN",2)){p+=2;while(*p&&isspace((unsigned char)*p))p++;if(*p!='(')return false;p++;
            int ch=atoi(p);while(*p&&*p!=',')p++;if(*p!=',')return false;p++;int var=atoi(p);
            while(*p&&*p!=')') p++;
            if(*p!=')'||k>=cap) return false;
            p++;
            out[k++]=(t425_occam_instr_t){OIN,(uint16_t)ch,(uint16_t)var,0};
        } else if(!strncmp(p,"END",3)){p+=3;if(k>=cap)return false;out[k++]=(t425_occam_instr_t){OEND,0,0,0};}
        else return false;
    }
    if(n) *n=k;
    return k>0;
}
