# V2.80 — memória

Expected RP2040 Pico standard 2 MiB Flash:

```text
FLASH physical: 0x200000
reserved config: 0x002000
firmware:        0x1FE000

start: 0x10000000
config: 0x101FE000 .. 0x10200000
```

The SDK's `pico_override_flash_size()` generates and registers the target's
`pico_flash_region.ld` override. citeturn0search0

T425 appends the configuration symbols to that generated script.

The final proof is the linker map.
