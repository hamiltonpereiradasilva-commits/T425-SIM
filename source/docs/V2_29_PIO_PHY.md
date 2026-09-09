# T425-SIM v2.29 — PIO PHY

A v2.29 introduces the first RP2040 PIO physical-layer slice.

## Bench topology

```text
P00 TX GPIO ─────────────────► P01 RX GPIO
P00 RX GPIO ◄───────────────── P01 TX GPIO
P00 GND     ────────────────── P01 GND
```

Recommended first bench test uses short wires and one dedicated GPIO pair per direction.

## Layering

```text
Application
    ↓
Transport + CRC
    ↓
Reliable Link
    ↓
PIO PHY
    ↓
GPIO
```

The PIO layer is intentionally isolated. It accepts the already serialized Transport frame; it does not interpret routing, ACKs, or OCCAM channels.

## Important validation boundary

v2.29 adds the RP2040-facing API and PIO program, but this environment does not have a connected RP2040 board available for electrical validation. Therefore this version is **source-integrated, not hardware-validated**.

The RX path remains a stable API placeholder until the TX waveform and framing are validated on the bench. Do not claim a P00↔P01 physical link is operational until that test is performed.
