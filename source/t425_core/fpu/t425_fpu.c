#include "t425_fpu.h"

void t425_fpu_reset(
    t425_fpu_state_t *fpu)
{
    if (!fpu)
        return;

    *fpu = (t425_fpu_state_t){0};
}
