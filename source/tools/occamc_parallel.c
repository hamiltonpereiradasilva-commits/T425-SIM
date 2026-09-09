#include <stdio.h>
#include <stdlib.h>
#include "t425_occam_text.h"
#include "t425_parallel_vm.h"
int main(int argc,char**argv){
 if(argc!=2){fprintf(stderr,"usage: occamc_parallel file.occ\n");return 2;}
 FILE*f=fopen(argv[1],"rb");if(!f)return 2;
 fseek(f,0,SEEK_END);long z=ftell(f);rewind(f);char*b=malloc((size_t)z+1);
 if(!b){fclose(f);return 2;}fread(b,1,(size_t)z,f);fclose(f);b[z]=0;
 t425_ast_t a;t425_parallel_vm_t v;
 if(!t425_occam_parse_text(b,&a)||!t425_parallel_vm_init(&v,&a)){
   fprintf(stderr,"OCCAM parallel compile error\n");free(b);return 1;
 }
 printf("processes=%u instructions=%u\n",v.image.process_count,v.image.ir_count);
 for(unsigned i=0;i<v.image.process_count;i++)
   printf("P%02u offset=%u count=%u\n",v.image.process[i].pid,
          v.image.process[i].ir_offset,v.image.process[i].ir_count);
 free(b);return 0;
}
