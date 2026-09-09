#!/usr/bin/env bash
set -euo pipefail
find build -type f \( -name '*.elf' -o -name '*.map' -o -name '*.bin' -o -name '*.uf2' \) -print0 |
  sort -z | xargs -0 shasum -a 256
