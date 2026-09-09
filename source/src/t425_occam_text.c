#include "t425_occam_text.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static void ws(const char**p){
    while(**p&&isspace((unsigned char)**p)) (*p)++;
}
static bool word(const char**p,const char*w){
    size_t n;
    ws(p);
    n=strlen(w);
    if(strncmp(*p,w,n)!=0) return false;
    *p+=n;
    return true;
}
static bool integer(const char**p,int*v){
    char*e;
    long x;
    ws(p);
    x=strtol(*p,&e,10);
    if(e==*p) return false;
    *p=e;
    *v=(int)x;
    return true;
}
static bool idnum(const char**p,char prefix,int*v){
    ws(p);
    if(**p!=prefix) return false;
    (*p)++;
    return integer(p,v);
}
static int add(t425_ast_t*a,t425_ast_kind_t k){
    t425_ast_node_t n={0};
    n.kind=k;
    return t425_ast_add(a,n);
}
static bool block(const char**p,t425_ast_t*a,int*root){
    t425_ast_kind_t k;
    int r;
    if(word(p,"SEQ")) k=AST_SEQ;
    else if(word(p,"PAR")) k=AST_PAR;
    else return false;
    ws(p);
    if(**p!='{') return false;
    (*p)++;
    r=add(a,k);
    if(r<0) return false;
    for(;;){
        int node=-1;
        ws(p);
        if(**p=='}'){(*p)++;break;}
        if(!**p) return false;
        if(word(p,"OUT")){
            int ch,val;
            ws(p); if(**p!='(') return false; (*p)++;
            if(!idnum(p,'c',&ch)) return false;
            ws(p); if(**p!=',') return false; (*p)++;
            if(!integer(p,&val)) return false;
            ws(p); if(**p!=')') return false; (*p)++;
            ws(p); if(**p!=';') return false; (*p)++;
            node=add(a,AST_OUT);
            if(node>=0){a->node[node].channel=(uint16_t)ch;a->node[node].value=val;}
        } else if(word(p,"IN")){
            int ch,var;
            ws(p); if(**p!='(') return false; (*p)++;
            if(!idnum(p,'c',&ch)) return false;
            ws(p); if(**p!=',') return false; (*p)++;
            if(!idnum(p,'v',&var)) return false;
            ws(p); if(**p!=')') return false; (*p)++;
            ws(p); if(**p!=';') return false; (*p)++;
            node=add(a,AST_IN);
            if(node>=0){a->node[node].channel=(uint16_t)ch;a->node[node].var=(uint16_t)var;}
        } else if(word(p,"TIMER_AFTER")){
            int timer;
            ws(p); if(**p!='(') return false; (*p)++;
            if(!integer(p,&timer)) return false;
            ws(p); if(**p!=')') return false; (*p)++;
            ws(p); if(**p!=';') return false; (*p)++;
            node=add(a,AST_TIMER_AFTER);
            if(node>=0) a->node[node].value=timer;
        } else if(word(p,"END")){
            ws(p); if(**p!=';') return false;
            (*p)++;
            node=add(a,AST_END);
        } else {
            int sub;
            if(!block(p,a,&sub)) return false;
            node=sub;
        }
        if(node<0) return false;
        if(!t425_ast_add_child(a,(uint16_t)r,(uint16_t)node)) return false;
    }
    *root=r;
    return true;
}
bool t425_occam_parse_text(const char*text,t425_ast_t*a){
    const char*p;
    int root;
    if(!text||!a) return false;
    t425_ast_init(a);
    p=text;
    for(;;){
        ws(&p);
        if(!*p) break;
        if(word(&p,"CHAN")){
            int n;
            ws(&p);
            if(!word(&p,"OF")) return false;
            ws(&p);
            if(!word(&p,"INT")) return false;
            ws(&p);
            if(!idnum(&p,'c',&n)) return false;
            ws(&p);
            if(*p==';') p++;
            continue;
        }
        if(!block(&p,a,&root)) return false;
        a->root=(uint16_t)root;
        break;
    }
    ws(&p);
    return *p=='\0';
}
