#pragma once

#include <stdint.h>

#ifndef T425_NODE_ID
#define T425_NODE_ID 0
#endif

#define T425_RAM_BYTES          4096u
#define T425_WORDS              (T425_RAM_BYTES / 4u)
#define T425_MAX_PROCESSES      16u
#define T425_MAX_CHANNELS       16u
#define T425_LINKS              4u

/* T425 internal RAM address range documented by the device manual. */
#define T425_INT_RAM_BASE       0x80000000u
#define T425_INT_RAM_END        0x80000FFFu
#define T425_MEMSTART           0x80000070u

/* Phase-1 management protocol */
#define T425_MGMT_MAGIC0        0x54u
#define T425_MGMT_MAGIC1        0x34u
#define T425_MGMT_VERSION       1u

/* The physical PIO link implementation is intentionally a lab transport
   in phase 1. The ISA/VM remains T425-oriented; link electrical/timing
   compatibility is a later validation stage. */
#define T425_LINK_BAUD         1000000u

#define T425_MGMT_UART          uart0
#define T425_MGMT_BAUD          115200u
#define T425_MGMT_TX_PIN        16u
#define T425_MGMT_RX_PIN        17u
#define T425_MGMT_DE_PIN        18u

#define T425_LED_PIN            25u
