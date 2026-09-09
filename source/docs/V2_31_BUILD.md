# Build v2.31

O target CMake deve chamar:

```cmake
pico_generate_pio_header(t425_firmware ${CMAKE_CURRENT_LIST_DIR}/t425_pio_tx.pio)
pico_generate_pio_header(t425_firmware ${CMAKE_CURRENT_LIST_DIR}/t425_pio_rx.pio)
```

e incluir `t425_pio_phy.c` no target.

Não editar manualmente os `.pio.h` gerados.
