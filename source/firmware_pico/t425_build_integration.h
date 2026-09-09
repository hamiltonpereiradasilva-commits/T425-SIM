#pragma once

/*
 * v2.64 build integration contract.
 *
 * The build system must define these from the authoritative board/linker setup.
 * No hard-coded RP2040 Flash address is accepted here.
 */
#ifndef T425_FLASH_TOTAL_BYTES
#error "Define T425_FLASH_TOTAL_BYTES from the selected RP2040 target."
#endif

#ifndef T425_APP_FLASH_BYTES
#error "Define T425_APP_FLASH_BYTES from the authoritative linker layout."
#endif

#define T425_CONFIG_BYTES 8192u

#if (T425_APP_FLASH_BYTES + T425_CONFIG_BYTES) > T425_FLASH_TOTAL_BYTES
#error "T425 application plus persistent configuration exceeds Flash capacity."
#endif
