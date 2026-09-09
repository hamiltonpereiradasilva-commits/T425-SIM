#include <stdio.h>
#include <stdlib.h>
#include "t425_occam_text.h"
#include "t425_occam_semantic.h"
#include "t425_occam_compile.h"
int main(int argc,char**argv){
    if(argc!=2){fprintf(stderr,"usage: occamc file.occ\n");return 2;}
    FILE*f=fopen(argv[1],"rb");if(!f){perror(argv[1]);return 2;}
    fseek(f,0,SEEK_END);long sz=ftell(f);rewind(f);char*b=malloc((size_t)sz+1);
    if(!b){fclose(f);return 2;}fread(b,1,(size_t)sz,f);fclose(f);b[sz]=0;
    t425_ast_t a;t425_occam_instr_t code[T425_OCCAM_MAX_OPS];uint16_t n;
    if(!t425_occam_parse_text(b,&a)||!t425_occam_semantic_check(&a)||!t425_occam_compile(&a,code,T425_OCCAM_MAX_OPS,&n)){
        fprintf(stderr,"OCCAM subset compile error\n");free(b);return 1;}
    printf("T425-IR %u instructions\n",n);
    for(uint16_t i=0;i<n;i++)printf("%u %u %u %u\n",i,code[i].op,code[i].arg0,code[i].imm);
    free(b);return 0;
}
