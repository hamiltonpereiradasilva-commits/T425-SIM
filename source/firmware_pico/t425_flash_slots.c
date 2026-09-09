#include "t425_flash_slots.h"

bool t425_flash_slot_valid(const t425_flash_slot_t *s) {
    return s && t425_config_validate(&s->record);
}

t425_slot_id_t t425_flash_select_active(const t425_flash_image_t *img) {
    if (!img) return T425_SLOT_NONE;
    bool va=t425_flash_slot_valid(&img->a);
    bool vb=t425_flash_slot_valid(&img->b);

    if (va && vb)
        return (img->a.generation >= img->b.generation)
               ? T425_SLOT_A : T425_SLOT_B;
    if (va) return T425_SLOT_A;
    if (vb) return T425_SLOT_B;
    return T425_SLOT_NONE;
}

bool t425_flash_prepare_commit(const t425_flash_image_t *current,
                               const t425_config_record_t *new_record,
                               t425_flash_image_t *next,
                               t425_slot_id_t *target_slot) {
    if (!current || !new_record || !next || !target_slot ||
        !t425_config_validate(new_record))
        return false;

    *next=*current;
    t425_slot_id_t active=t425_flash_select_active(current);

    if (active == T425_SLOT_A) {
        if (current->a.generation == UINT32_MAX) return false;
        *target_slot=T425_SLOT_B;
        next->b.record=*new_record;
        next->b.generation=current->a.generation+1;
    } else if (active == T425_SLOT_B) {
        if (current->b.generation == UINT32_MAX) return false;
        *target_slot=T425_SLOT_A;
        next->a.record=*new_record;
        next->a.generation=current->b.generation+1;
    } else {
        *target_slot=T425_SLOT_A;
        next->a.record=*new_record;
        next->a.generation=1;
    }
    return true;
}
