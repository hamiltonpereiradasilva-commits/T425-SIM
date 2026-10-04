#ifndef T425_RUNTIME_H
#define T425_RUNTIME_H
#include <stdbool.h>
#include <stdint.h>
#include "t425_core_phy_bootstrap.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct
{
    t425_core_phy_bootstrap_t bootstrap;
    bool initialized;
} t425_runtime_t;
bool t425_runtime_init(t425_runtime_t *runtime, const t425_core_phy_ops_t *core1_ops);
t425_core_phy_t *t425_runtime_core0_phy(t425_runtime_t *runtime);
uint32_t t425_runtime_poll_core1(t425_runtime_t *runtime);
#ifdef __cplusplus
}
#endif
#endif
