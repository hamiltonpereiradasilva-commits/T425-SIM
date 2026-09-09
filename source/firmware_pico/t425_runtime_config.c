#include "t425_runtime_config.h"
#include "t425_flash_hw.h"
#include <string.h>

/*
 * Boot loader policy:
 *  1. read both persistent slots;
 *  2. validate each;
 *  3. select highest valid generation;
 *  4. expose NODE_ID only after validation.
 *
 * The hardware read binding remains isolated from this policy.
 */
bool t425_runtime_config_load(t425_runtime_config_t *rt) {
    if (!rt) return false;
    memset(rt,0,sizeof(*rt));

    t425_flash_image_t image;
    if (!t425_flash_hw_boot(&image,(t425_slot_id_t *)&rt->active_slot)) {
        rt->state=T425_BOOT_RECOVERY;
        return false;
    }

    t425_slot_id_t active=t425_flash_select_active(&image);
    if (active == T425_SLOT_NONE) {
        rt->state=T425_BOOT_RECOVERY;
        return false;
    }

    const t425_flash_slot_t *s =
        (active == T425_SLOT_A) ? &image.a : &image.b;

    rt->config=s->record;
    rt->active_slot=(uint8_t)active;
    rt->state=T425_BOOT_CONFIGURED;
    return t425_runtime_config_validate(rt);
}

bool t425_runtime_config_validate(const t425_runtime_config_t *rt) {
    if (!rt || rt->state != T425_BOOT_CONFIGURED) return false;
    return rt->config.node_id <= 255;
}
