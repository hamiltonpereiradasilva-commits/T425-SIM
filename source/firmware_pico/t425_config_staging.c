#include "t425_config_staging.h"
#include <string.h>

void t425_config_staging_clear(t425_config_staging_t *s) {
    if (s) memset(s,0,sizeof(*s));
}

bool t425_config_staging_set(t425_config_staging_t *s,
                             const uint8_t *payload, uint16_t len) {
    if (!s || !payload || len != sizeof(t425_config_record_t)) return false;
    t425_config_record_t r;
    memcpy(&r,payload,sizeof(r));

    if (!t425_config_validate(&r)) return false;
    s->record=r;
    s->present=true;
    return true;
}

bool t425_config_staging_get(const t425_config_staging_t *s,
                             uint8_t *payload, uint16_t cap, uint16_t *len) {
    if (!s || !s->present || !payload || !len ||
        cap < sizeof(t425_config_record_t)) return false;
    memcpy(payload,&s->record,sizeof(s->record));
    *len=(uint16_t)sizeof(s->record);
    return true;
}

bool t425_config_staging_valid(const t425_config_staging_t *s) {
    return s && s->present && t425_config_validate(&s->record);
}
