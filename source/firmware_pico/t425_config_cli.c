#include "t425_config_cli.h"
#include <string.h>
#include <stdlib.h>

static bool tok(char **p, char **out) {
    while (**p == ' ' || **p == '\t') (*p)++;
    if (!**p) return false;
    *out = *p;
    while (**p && **p != ' ' && **p != '\t' && **p != '\r' && **p != '\n') (*p)++;
    if (**p) *(*p)++ = 0;
    return true;
}

t425_cli_status_t t425_cli_execute(char *line,
                                    t425_node_config_t *cfg,
                                    bool *commit_requested) {
    if (!line || !cfg || !commit_requested) return T425_CLI_ERR_STATE;
    *commit_requested = false;

    char *p=line, *a=NULL, *b=NULL, *c=NULL, *d=NULL, *e=NULL;
    if (!tok(&p,&a)) return T425_CLI_ERR_SYNTAX;

    if (!strcmp(a,"GET_CONFIG")) return T425_CLI_OK;

    if (!strcmp(a,"SET_NODE_ID")) {
        if (!tok(&p,&b)) return T425_CLI_ERR_SYNTAX;
        long n=strtol(b,NULL,10);
        if (n < 0 || n > 255) return T425_CLI_ERR_RANGE;
        cfg->node_id=(uint8_t)n;
        return T425_CLI_OK;
    }

    if (!strcmp(a,"SET_INTERFACE")) {
        if (!tok(&p,&b) || !tok(&p,&c) || !tok(&p,&d) || !tok(&p,&e))
            return T425_CLI_ERR_SYNTAX;
        long i=strtol(b,NULL,10), peer=strtol(c,NULL,10);
        long sm=strtol(d,NULL,10), tx=strtol(e,NULL,10);
        char *f=NULL;
        if (!tok(&p,&f)) return T425_CLI_ERR_SYNTAX;
        long rx=strtol(f,NULL,10);

        if (i < 0 || i >= T425_MAX_INTERFACES ||
            peer < 0 || peer > 255 ||
            sm < 0 || sm > 255 ||
            tx < 0 || tx > 255 ||
            rx < 0 || rx > 255)
            return T425_CLI_ERR_RANGE;

        if (i >= cfg->interfaces.count) {
            if (i != cfg->interfaces.count) return T425_CLI_ERR_RANGE;
            if (!t425_interface_add(&cfg->interfaces,(uint8_t)peer,
                                    (uint8_t)sm,(uint8_t)tx,(uint8_t)rx))
                return T425_CLI_ERR_RANGE;
        } else {
            cfg->interfaces.items[i].peer_id=(uint8_t)peer;
            cfg->interfaces.items[i].pio_sm=(uint8_t)sm;
            cfg->interfaces.items[i].tx_gpio=(uint8_t)tx;
            cfg->interfaces.items[i].rx_gpio=(uint8_t)rx;
            cfg->interfaces.items[i].state=T425_IF_UP;
        }
        return T425_CLI_OK;
    }

    if (!strcmp(a,"COMMIT")) {
        *commit_requested=true;
        return T425_CLI_OK;
    }

    if (!strcmp(a,"VALIDATE")) return T425_CLI_OK;

    return T425_CLI_ERR_SYNTAX;
}
