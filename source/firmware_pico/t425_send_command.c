#include "t425_send_command.h"
#include <stdlib.h>
#include <string.h>
bool t425_send_command(t425_application_t*a,const char*line,uint32_t now){
    if(!a||!line||strncmp(line,"SEND ",5))return false;
    const char*p=line+5;char*end=0;long d=strtol(p,&end,10);
    if(end==p||d<0||d>255||*end!=' ')return false;
    const char*txt=end+1;size_t n=strlen(txt);
    if(n>T425_ROUTE_MAX_PAYLOAD)return false;
    return t425_app_send(a,(uint8_t)d,(const uint8_t*)txt,(uint8_t)n,now);
}
