#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_node_config.h"

typedef enum {
    T425_CLI_OK = 0,
    T425_CLI_ERR_SYNTAX = 1,
    T425_CLI_ERR_RANGE = 2,
    T425_CLI_ERR_STATE = 3
} t425_cli_status_t;

t425_cli_status_t t425_cli_execute(char *line,
                                    t425_node_config_t *cfg,
                                    bool *commit_requested);
