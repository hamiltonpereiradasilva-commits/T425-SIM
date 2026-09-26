#ifndef T425_INCREMENTOR_H
#define T425_INCREMENTOR_H

#include <stdint.h>

typedef struct
{
    uint8_t inc_oreg_carryout;
    uint8_t inc_dreg_carryout;
} t425_incrementor_result_t;

void t425_incrementor_reset(t425_incrementor_result_t *result);

void t425_incrementor_word_oreg(
    uint32_t *oreg,
    t425_incrementor_result_t *result
);

void t425_incrementor_word_dreg(
    uint32_t *dreg,
    t425_incrementor_result_t *result
);

#endif
