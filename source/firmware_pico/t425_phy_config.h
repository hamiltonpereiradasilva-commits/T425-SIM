#pragma once

#define T425_PHY_BITS_PER_SYMBOL 8u
#define T425_PHY_DATA_BITS 8u
#define T425_PHY_STOP_BITS 1u
#define T425_PHY_PARITY_BITS 0u

/* Number of PIO clock cycles occupied by one serial bit in v2.33. */
#define T425_PHY_PIO_CYCLES_PER_BIT 8u

#define T425_PHY_DEFAULT_BAUD 115200u
#define T425_PHY_DEFAULT_TX_PIN 2u
#define T425_PHY_DEFAULT_RX_PIN 3u
