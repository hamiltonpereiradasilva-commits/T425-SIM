#!/usr/bin/env bash
set -euo pipefail
echo "=== T425 v2.87 BUILD GATE ==="
command -v cmake >/dev/null 2>&1 || { echo "ERROR: cmake missing"; exit 10; }
command -v arm-none-eabi-gcc >/dev/null 2>&1 || { echo "ERROR: arm-none-eabi-gcc missing; no ARM build performed"; exit 11; }
test -f pico_sdk_import.cmake || { echo "ERROR: pico_sdk_import.cmake missing"; exit 13; }
test -n "${PICO_SDK_PATH:-}" && test -f "${PICO_SDK_PATH}/pico_sdk_init.cmake" || { echo "ERROR: invalid PICO_SDK_PATH"; exit 12; }
rm -rf build
cmake -S . -B build -DPICO_BOARD=pico -DPICO_SDK_PATH="${PICO_SDK_PATH}"
cmake --build build --parallel
MAP="$(find build -type f -name '*.map' -print -quit)"
ELF="$(find build -type f -name '*.elf' -print -quit)"
UF2="$(find build -type f -name '*.uf2' -print -quit)"
test -n "$MAP" && test -n "$ELF" && test -n "$UF2" || { echo "ERROR: required artifacts missing"; exit 20; }
grep -E '__t425_config_(start|end)__' "$MAP"
printf '%s\n' "$ELF" "$MAP" "$UF2"
