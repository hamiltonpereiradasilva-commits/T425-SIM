# V2.80 — primeiro build real

Prerequisites:

- Pico SDK;
- ARM GNU toolchain;
- CMake.

Expected configuration:

```bash
cmake -S firmware_pico -B build -DPICO_BOARD=pico
```

Expected build:

```bash
cmake --build build -j
```

Expected outputs:

```text
t425_pico_v15.elf
t425_pico_v15.map
t425_pico_v15.bin
t425_pico_v15.uf2
```

The SDK documents `pico_add_extra_outputs()` as generating MAP/BIN/UF2 in
addition to ELF. citeturn0search5turn0search9

After the build, the `.map` must be inspected rather than trusting the CMake
configuration.
