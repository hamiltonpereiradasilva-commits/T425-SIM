#include "t425_config_canonical.h"
#include "t425_config_store.h"

bool t425_config_canonical_validate(const t425_config_record_t *record) {
    return t425_config_validate(record);
}
