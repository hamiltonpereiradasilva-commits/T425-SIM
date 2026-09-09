# T425-SIM v2.80 — first-build audit

A source-level build gate was performed before attempting the RP2040 build.

## Defects corrected

### 1. `main.c` used an obsolete configuration API

The current `t425_node_config_t` contains:

```text
node_id
interfaces
```

not the old `links[]` structure.

`main.c` was still using:

```text
t425_node_config_add_link()
cfg.links[]
neighbor_id
tx_pin
rx_pin
pio_index
```

Those names do not belong to the current header contract.

It now uses:

```text
t425_node_config_add_interface()
cfg.interfaces.items[]
peer_id
tx_gpio
rx_gpio
pio_sm
```

### 2. Missing `t425_interfaces.c`

`t425_node_config.c` calls:

```text
t425_interfaces_init()
t425_interface_add()
```

but `t425_interfaces.c` was not in the CMake target.

It is now explicitly compiled.

### 3. Host syntax gate

All CMake-listed sources that do not require Pico SDK headers were passed through:

```text
gcc -std=c11 -Wall -Wextra -Werror -fsyntax-only
```

No errors were found in that subset.

This is not a substitute for the ARM/Pico build; it is an additional static gate.

## Important conclusion

The source tree still requires a real Pico SDK + ARM GCC build before a UF2 can be declared valid.
