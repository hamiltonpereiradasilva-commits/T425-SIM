#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "t425_node_config.h"

#define T425_CONFIG_MAGIC   0x54343235u
#define T425_CONFIG_VERSION 1u

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t length;
    t425_node_config_t config;
    uint32_t crc32;
} t425_config_record_t;

bool t425_config_validate(const t425_config_record_t *r);
bool t425_config_build(t425_config_record_t *r,
                       const t425_node_config_t *cfg);

/* Persistent A/B store API used by the runtime and console. */
bool t425_config_store_load(t425_node_config_t *cfg);
bool t425_config_store_save(const t425_node_config_t *cfg);
bool t425_config_store_clear(void);
