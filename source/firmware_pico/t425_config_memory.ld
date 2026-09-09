/*
 * T425 persistent configuration reservation.
 *
 * This fragment is included by the application's linker script.
 * It reserves exactly 8 KiB at the top of the selected Flash image.
 *
 * IMPORTANT: FLASH_LENGTH must match the board/application linker definition.
 */
#ifndef T425_CONFIG_REGION_SIZE
#define T425_CONFIG_REGION_SIZE 8192
#endif

/* Conceptual fragment:
 *
 *   FLASH (rx) : ORIGIN = FLASH_ORIGIN, LENGTH = FLASH_LENGTH - T425_CONFIG_REGION_SIZE
 *
 *   .t425_config (NOLOAD) :
 *   {
 *       __t425_config_start__ = ORIGIN(FLASH) + LENGTH(FLASH);
 *       __t425_config_end__   = __t425_config_start__ + T425_CONFIG_REGION_SIZE;
 *   }
 *
 * The exact syntax must be merged into the project's authoritative linker
 * script rather than maintained as a second independent memory map.
 */
