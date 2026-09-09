#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_router.h"

typedef struct {
    t425_router_t *router;
    uint8_t default_ttl;
    uint8_t next_sequence;
} t425_console_t;

void t425_console_init(t425_console_t *c, t425_router_t *router);
bool t425_console_execute(t425_console_t *c, const char *line);
void t425_console_print_help(void);
void t425_console_print_routes(const t425_router_t *r);
void t425_console_print_stats(const t425_router_t *r);
