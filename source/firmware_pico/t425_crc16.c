#include "t425_crc16.h"

/*
 * CRC-16/CCITT-FALSE
 *
 * Polynomial : 0x1021
 * Initial    : 0xFFFF
 * RefIn      : false
 * RefOut     : false
 * XorOut     : 0x0000
 *
 * This CRC is used by the experimental UART/PIO transport
 * protocol and is intentionally separate from the canonical
 * T425 core CRC implementation.
 */

uint16_t t425_crc16_ccitt(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFFu;

    for (uint16_t i = 0; i < len; ++i) {

        crc ^= (uint16_t)data[i] << 8;

        for (uint8_t bit = 0; bit < 8; ++bit) {

            crc = (crc & 0x8000u)
                ? (uint16_t)((crc << 1) ^ 0x1021u)
                : (uint16_t)(crc << 1);
        }
    }

    return crc;
}
