#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "t425_config_cli.h"

static void run(t425_node_config_t *cfg, const char *s) {
    char line[96];
    bool commit=false;
    strncpy(line,s,sizeof(line)-1); line[sizeof(line)-1]=0;
    t425_cli_status_t st=t425_cli_execute(line,cfg,&commit);
    printf("CMD=%s STATUS=%u COMMIT=%u NODE=%u IF=%u\r\n",
           s, st, commit, cfg->node_id, cfg->interfaces.count);
}

int main(void) {
    stdio_init_all(); sleep_ms(700);

    t425_node_config_t cfg;
    t425_node_config_init(&cfg,0);

    printf("T425 USB CONFIG CLI V2.58\r\n");
    run(&cfg,"GET_CONFIG");
    run(&cfg,"SET_NODE_ID 4");
    run(&cfg,"SET_INTERFACE 0 1 0 2 3");
    run(&cfg,"SET_INTERFACE 1 2 1 4 5");
    run(&cfg,"VALIDATE");
    run(&cfg,"COMMIT");

    while (1) sleep_ms(1000);
}
