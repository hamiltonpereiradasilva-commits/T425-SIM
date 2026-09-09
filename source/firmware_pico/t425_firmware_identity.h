#pragma once
#include <stdint.h>

/*
 * Identity is runtime configuration, not a build identity.
 * The same UF2 may therefore be installed on P00, P01, P02, P03, ...
 */
typedef struct {
    uint8_t node_id;
    uint32_t firmware_abi;
} t425_firmware_identity_t;

#define T425_FIRMWARE_ABI 1u
