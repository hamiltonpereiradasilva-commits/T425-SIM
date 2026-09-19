#include "t425_microsequencer.h"

#include <stddef.h>

#include "t425_crom.h"
#include "t425_mir_fields.h"

void t425_microsequencer_reset(
    t425_microsequencer_t *seq,
    uint16_t initial_mpc)
{
    if (seq == NULL)
        return;

    seq->mpc = (uint16_t)(initial_mpc & 0x03FFu);
    seq->next_base = 0u;

    t425_mir_load(&seq->mir, seq->mpc);
}

void t425_microsequencer_load_mir(
    t425_microsequencer_t *seq)
{
    if (seq == NULL)
        return;

    seq->mpc &= 0x03FFu;

    t425_mir_load(&seq->mir, seq->mpc);
}

uint16_t t425_microsequencer_next_base(
    const t425_microsequencer_t *seq)
{
    if (seq == NULL)
        return 0u;

    return (uint16_t)(t425_mir_next_field(&seq->mir) & 0x03FFu);
}

void t425_microsequencer_advance_base(
    t425_microsequencer_t *seq)
{
    if (seq == NULL)
        return;

    seq->next_base = t425_microsequencer_next_base(seq);
    seq->mpc = seq->next_base;

    t425_microsequencer_load_mir(seq);
}
