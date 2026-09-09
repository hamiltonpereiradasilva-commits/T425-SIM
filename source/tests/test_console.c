#include <assert.h>
#include <stdio.h>
#include "t425_console.h"
#include "t425_topology.h"
int main(void){
 t425_router_t r;t425_console_t c;
 t425_router_init(&r,0);t425_topology_linear(&r);t425_console_init(&c,&r);
 assert(t425_console_execute(&c,"STATUS"));
 assert(t425_console_execute(&c,"ROUTES"));
 assert(t425_console_execute(&c,"PING 3"));
 assert(t425_console_execute(&c,"SEND 3 HELLO"));
 assert(!t425_console_execute(&c,"SEND 3"));
 assert(!t425_console_execute(&c,"BOGUS"));
 puts("T425 USB CONSOLE: PASS");return 0;
}
