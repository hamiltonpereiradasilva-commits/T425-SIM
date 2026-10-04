# F_PHY-70B - CORE/PHY link seam

The canonical CORE has a non-owning optional `t425_core_phy_t *`
attachment. Boundary helpers are implemented in `t425_core.c`:

- `t425_core_link_output(core, link_id)` sends the current
  `t425_channel_data_reg` byte to endpoint `link_id`.
- `t425_core_link_input_available(core, link_id)` is non-consuming.
- `t425_core_link_input_consume(core, link_id, byte)` consumes one frame
  and stores the byte in `t425_channel_data_reg`.

LinkN maps directly to endpoint N for N=0..3. `t425_channel_select` is
never used for endpoint selection. Frames wider than one byte are
rejected instead of silently truncated.

The attachment is optional and non-owning. No queue, FIFO, retry,
timeout, physical ACK state, RP2040 dependency, PIO, DMA, GPIO, IRQ,
or speed-selection signal is added.
