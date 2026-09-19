#ifndef T425_CROM_H
#define T425_CROM_H

#include <stdint.h>

#define T425_CROM_ROWS      1024u
#define T425_CROM_BITS      118u
#define T425_CROM_TOTAL_BITS (T425_CROM_ROWS * T425_CROM_BITS)

/*
 * IMS T425 CROM.
 *
 * This is a literal architectural microcode image imported from
 * t425ROM.h.  The representation preserves all 1024 rows and
 * all 118 bits per row.
 *
 * No interpretation of the fields is performed here.
 */
extern const uint8_t
t425_crom[T425_CROM_ROWS][T425_CROM_BITS];

#endif /* T425_CROM_H */
