# T425-SIM v2.82 — canonical configuration contract

The v2.81 audit exposed a more fundamental problem: there were multiple
`t425_node_config.h/.c` implementations in `src`, `include`, and
`firmware_pico`, with incompatible structures.

That is not acceptable for a real build.

## Correction

The current interface-based contract is now canonical:

```text
t425_node_config_t
    node_id
    interfaces
```

with:

```text
t425_node_config_init()
t425_node_config_add_interface()
```

The same header/implementation is synchronized across the source/include
boundaries, and CMake uses the canonical `src/t425_node_config.c`.

This eliminates the old `links[]`, `node_count_hint`, and `link_baud` fields.

The physical baud remains a build/runtime hardware parameter
(`T425_LINK_BAUD`) rather than persistent node configuration.
