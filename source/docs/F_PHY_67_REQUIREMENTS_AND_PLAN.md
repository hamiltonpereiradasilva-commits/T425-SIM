# F_PHY-67 Requirements and Plan

## Closed baseline

F_PHY-63, F_PHY-64, F_PHY-65 and F_PHY-66 are CLOSED.

The CORE remains RP2040-independent. t425_channel_select remains architectural,
endpoint_id remains service-level, architectural OUT completion is unresolved,
the neutral runtime remains separate from physical ownership, Core1 launch is
deferred, and t425_core_engine remains unresolved.

## Objective

F_PHY-67 — RP2040 PHY API and lifecycle consistency gate.

Make the existing t425_link API internally consistent and make local DMA
lifecycle cleanup coherent without changing IMS physical semantics.

## Evidence inspected

- source/t425_phy/rp2040/t425_link.h
- source/t425_phy/rp2040/t425_link.c
- source/t425_phy/rp2040/t425_link.pio
- source/t425_phy/rp2040/t425_link_rx.pio
- source/t425_phy/rp2040/CMakeLists.txt
- source/t425_phy/rp2040/core_adapter/*
- source/t425_core/phy/*
- source/t425_runtime/*

The authoritative IMS contract remains the F_PHY-66 contract. No PIO, GPIO,
ACK scheduling or DMA architecture decision is made here.

## Public API matrix

| Symbol | Header | Source | Caller | Finding |
|---|---|---|---|---|
| t425_link_init | bool | bool | four-link initializer | consistent |
| t425_link_dma_init | bool | bool | four-link initializer | consistent |
| t425_link_dma_deinit | void | missing | four-link failure path | missing |
| t425_link_start | void | void | RP2040 adapter | consistent |
| t425_link_stop | void | void | RP2040 adapter | consistent |
| t425_link_send_frame | bool | void | rp2040_tx | mismatch |
| t425_link_send_ack | bool | void | ACK service paths | mismatch |
| t425_link_note_ack_received | void | missing | pump_rx | missing |
| t425_link_rx_available | bool | bool | rp2040_rx_available | consistent |
| t425_link_rx_get | uint32_t | uint32_t | rp2040_rx_get | consistent |

## Local acceptance contract

send_frame and send_ack return true only when local PHY submission is accepted
or started. They do not mean IMS ACK receipt, transmission completion,
architectural OUT completion or process rescheduling.

Existing data and ACK framing remains unchanged.

## ACK-state boundary

tx_waiting_ack is currently initialized nowhere, set nowhere, tested nowhere and
cleared nowhere. F_PHY-67 initializes it to false and makes
t425_link_note_ack_received clear only that field.

F_PHY-67 does not add a setter, transmission gate, retry, timeout, scheduling or
overlapped-ACK policy.

## DMA acquisition and rollback

The current sequence claims TX DMA, claims RX DMA, configures both channels and
stores the external RX buffer.

If a later claim fails, the already claimed channel is aborted and unclaimed.
Deinitialization aborts and unclaims each valid channel, sets IDs to -1, clears
buffer metadata and resets existing RX DMA flags/counters. The external buffer
is never freed.

Deinitialization is safe for initialized, partially initialized,
uninitialized and repeatedly deinitialized links.

The Pico SDK APIs confirmed for this plan are:

- dma_channel_abort(uint channel)
- dma_channel_unclaim(uint channel)
- dma_channel_cleanup(uint channel) exists but is not required here

RX DMA remains non-operational; no ring architecture or DREQ policy changes.

## PIO resource boundary

F_PHY-67 does not claim state machines, remove PIO programs, add a PIO program
manager, change generated headers or modify either PIO source. PIO program
lifetime and platform resource ownership remain deferred.

## Static-linkability blockers

F_PHY-67 fixes declaration/definition mismatches and missing local lifecycle
symbols. The physical CMake target still references unresolved
t425_core_engine and has an external dependency graph problem. ACK IRQ
production, RX DMA operation, GPIO evidence and Core1 remain runtime/physical
blockers and are not fixed here.

## Exact changeset

Modify only:

- source/t425_phy/rp2040/t425_link.h
- source/t425_phy/rp2040/t425_link.c

Create only:

- source/docs/F_PHY_67_REQUIREMENTS_AND_PLAN.md
- tests/t425_phy/test_f_phy_67_link_lifecycle.c
- tests/t425_phy/support/hardware/pio.h
- tests/t425_phy/support/hardware/dma.h
- tests/t425_phy/support/hardware/clocks.h
- tests/t425_phy/support/hardware/gpio.h
- tests/t425_phy/support/pico_queue_shim.c

The support shim supplies only host test doubles for PIO, DMA, clocks and GPIO.
It does not enter production source.

No CORE, runtime, adapter, PIO, generated header, GPIO assignment or CMake file
is changed.

## Test strategy

The strict host test verifies:

- public API compilation and link resolution;
- deterministic initialization;
- successful DMA setup;
- rollback after the second DMA claim fails;
- abort/unclaim on deinit;
- repeated deinit safety;
- reset of IDs, buffer metadata and DMA state;
- external RX buffer is not freed or modified;
- ACK notification clears only tx_waiting_ack;
- send_frame/send_ack indicate only local submission acceptance.

The test does not claim physical waveform correctness, hardware interoperability,
ACK IRQ correctness, operational RX DMA or architectural OUT completion.

## Baseline-aware protection

Before writing, the proposal records complete Git status and hashes of PIO,
generated PIO headers, CORE, runtime, adapter and CMake paths under /tmp.
After writing and testing, it compares those baseline records. It excludes only
the two authorized modified source files, the requirements document and the
tests/t425_phy test/shim paths.

## Explicit exclusions and deferred items

No PIO protocol or sampling changes, RX IRQ producer, overlapped ACK scheduling,
ACK retry policy, DMA architecture, operational RX DMA, GPIO assignment,
voltage translation, signal integrity, PIO SM allocation, PIO program manager,
four-link activation, Core1 launch, VM integration, architectural semantics or
t425_core_engine alias is included.

## Acceptance criteria

- all public declarations and definitions are consistent;
- no in-scope declared implementation is missing;
- DMA partial initialization rolls back;
- deinit is idempotent and does not leak channels;
- ACK notification only clears the established local state;
- Boolean send results mean local acceptance only;
- IMS framing is unchanged;
- PIO, generated PIO headers and GPIO mapping are unchanged;
- CORE, runtime, adapter and CMake are unchanged;
- Core1 remains deferred;
- t425_core_engine remains unresolved;
- strict isolated validation passes.

## GO/NO-GO

GO for a bounded F_PHY-67 implementation proposal limited to the files above.

NO-GO for physical protocol, GPIO, PIO, DMA architecture, Core1 or CORE work.
