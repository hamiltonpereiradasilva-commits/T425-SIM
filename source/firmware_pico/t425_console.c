#include "t425_console.h"
#include "t425_transport.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void t425_console_init(t425_console_t*c,t425_router_t*r){
    c->router=r;c->default_ttl=8;c->next_sequence=0;
}
void t425_console_print_help(void){
    puts("COMMANDS: HELP | STATUS | ROUTES | STATS | PING n | SEND n text");
}
void t425_console_print_routes(const t425_router_t*r){
    puts("ROUTES:");
    for(unsigned i=0;i<T425_ROUTE_MAX;i++) if(r->routes[i].valid)
        printf("DEST=%u NEXT=%u LINK=%u METRIC=%u\r\n",
               r->routes[i].destination,r->routes[i].next_hop,
               r->routes[i].link,r->routes[i].metric);
}
void t425_console_print_stats(const t425_router_t*r){
    printf("FORWARDED=%lu DELIVERED=%lu DROP_TTL=%lu DROP_NOROUTE=%lu\r\n",
           (unsigned long)r->forwarded,(unsigned long)r->delivered,
           (unsigned long)r->dropped_ttl,(unsigned long)r->dropped_no_route);
}
bool t425_console_execute(t425_console_t*c,const char*line){
    if(!c||!line)return false;
    while(*line==' ')line++;
    if(!strcmp(line,"HELP")){t425_console_print_help();return true;}
    if(!strcmp(line,"ROUTES")){t425_console_print_routes(c->router);return true;}
    if(!strcmp(line,"STATS")){t425_console_print_stats(c->router);return true;}
    if(!strcmp(line,"STATUS")){
        printf("NODE=%u TTL=%u NEXT_SEQ=%u\r\n",c->router->node_id,c->default_ttl,c->next_sequence);
        t425_console_print_stats(c->router);return true;
    }
    if(!strncmp(line,"PING ",5)){
        int n=atoi(line+5); if(n<0||n>255){puts("ERR DEST");return false;}
        printf("PING DEST=%d SEQ=%u\r\n",n,c->next_sequence++); return true;
    }
    if(!strncmp(line,"SEND ",5)){
        const char *p=line+5; char *end=0; long d=strtol(p,&end,10);
        if(end==p||d<0||d>255||*end!=' '){puts("ERR SEND");return false;}
        const char *text=end+1; size_t l=strlen(text);
        if(l>T425_ROUTE_MAX_PAYLOAD){puts("ERR PAYLOAD");return false;}
        t425_packet_t pkt={0};pkt.destination=(uint8_t)d;
        pkt.source=c->router->node_id;pkt.ttl=c->default_ttl;
        pkt.sequence=c->next_sequence++;pkt.length=(uint8_t)l;
        memcpy(pkt.payload,text,l);
        uint8_t wire[T425_WIRE_MAX];uint16_t n=t425_transport_encode(&pkt,wire,sizeof(wire));
        if(!n){puts("ERR ENCODE");return false;}
        printf("SEND DEST=%u SOURCE=%u SEQ=%u LEN=%u WIRE=%u\r\n",
               pkt.destination,pkt.source,pkt.sequence,pkt.length,n);
        return true;
    }
    puts("ERR UNKNOWN");return false;
}
