#include "t425_mir.h"

#include <stddef.h>

void
t425_mir_load(
    t425_mir_t *mir,
    uint16_t crom_index
)
{
    if (mir == NULL)
        return;

    if (crom_index >= T425_CROM_ROWS)
        return;

    for (unsigned i = 0u; i < T425_CROM_BITS; ++i)
        mir->bit[i] = t425_crom[crom_index][i];
}
