#include "t425_config_store.h"
#include "t425_flash_hw.h"
#include "t425_flash_transaction.h"

/* Deterministic CRC-32 implementation for the configuration record.
 * The CRC field itself is excluded from the calculation. */
static uint32_t crc32_update(uint32_t crc, uint8_t b) {
    crc ^= b;
    for (int i=0; i<8; ++i)
        crc = (crc & 1u) ? ((crc >> 1) ^ 0xEDB88320u) : (crc >> 1);
    return crc;
}

static uint32_t crc32_bytes(const uint8_t *p, uint32_t n) {
    uint32_t crc = 0xFFFFFFFFu;
    for (uint32_t i=0; i<n; ++i) crc = crc32_update(crc, p[i]);
    return ~crc;
}

bool t425_config_build(t425_config_record_t *r,
                       const t425_node_config_t *cfg) {
    if (!r || !cfg || !t425_node_config_validate(cfg)) return false;
    memset(r,0,sizeof(*r));
    r->magic = T425_CONFIG_MAGIC;
    r->version = T425_CONFIG_VERSION;
    r->length = (uint16_t)sizeof(t425_node_config_t);
    r->config = *cfg;
    r->crc32 = crc32_bytes((const uint8_t *)r,
                           (uint32_t)offsetof(t425_config_record_t, crc32));
    return true;
}

bool t425_config_validate(const t425_config_record_t *r) {
    if (!r || r->magic != T425_CONFIG_MAGIC ||
        r->version != T425_CONFIG_VERSION ||
        r->length != sizeof(t425_node_config_t))
        return false;

    uint32_t crc = crc32_bytes((const uint8_t *)r,
                               (uint32_t)offsetof(t425_config_record_t, crc32));
    return crc == r->crc32 && t425_node_config_validate(&r->config);
}


bool t425_config_store_load(t425_node_config_t *cfg) {
    if (!cfg) return false;

    t425_flash_image_t image;
    t425_slot_id_t active;
    if (!t425_flash_hw_boot(&image,&active)) return false;
    if (active == T425_SLOT_NONE) return false;

    const t425_flash_slot_t *slot =
        (active == T425_SLOT_A) ? &image.a : &image.b;
    if (!t425_flash_slot_valid(slot)) return false;

    *cfg=slot->record.config;
    return true;
}

bool t425_config_store_save(const t425_node_config_t *cfg) {
    if (!cfg) return false;

    t425_config_record_t record;
    if (!t425_config_build(&record,cfg)) return false;

    t425_flash_image_t current, after;
    t425_slot_id_t active;
    if (!t425_flash_hw_boot(&current,&active)) return false;

    t425_slot_id_t new_active=T425_SLOT_NONE;
    return t425_flash_transaction(&current,&record,&after,&new_active)
           == T425_FLASH_TX_OK;
}

bool t425_config_store_clear(void) {
    return t425_flash_hw_clear_all();
}
