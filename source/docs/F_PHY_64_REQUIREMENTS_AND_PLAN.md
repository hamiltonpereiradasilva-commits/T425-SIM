# F_PHY-64 — CORE/Link Service Ownership and Binding

## Baseline

F_PHY-63 is closed. The validated portable contract is:

- source/t425_core/phy/t425_core_phy.h
- source/t425_core/phy/t425_core_phy.c

The contract owns callback attachment, endpoint validation, byte-boundary TX
validation, callback dispatch and callback return-code propagation. It does
not own physical links, IPC, PIO, DMA, GPIO, IRQ, multicore state, FIFO
semantics, rendezvous state or architectural OUT completion.

## Current Evidence

The current RP2040 IPC implementation provides t425_core_phy_ipc_t with four
queues in each direction, t425_core_phy_ipc_make_core0_ops(),
t425_core_phy_ipc_service_init() and t425_core_phy_ipc_service_poll().

The current RP2040 adapter provides t425_core_phy_rp2040_t,
t425_core_phy_rp2040_make_ops() and explicit logical endpoint to
physical-link routes.

No current source owns a Core0 t425_core_phy_t. No current Core1 entrypoint,
multicore launch, service loop caller or canonical runtime binding exists.

## Objective

F_PHY-64 creates one explicit ownership and binding seam for the existing
Core0/Core1 Link Service plumbing. It does not integrate the architectural
CORE and does not implement physical PHY behavior.

## Ownership Graph

The bootstrap owns t425_core_phy_ipc_t ipc, Core0 t425_core_phy_t core0_phy,
t425_core_phy_ipc_service_t core1_service and initialized state.

The platform runtime owns t425_core_phy_rp2040_t and supplies its callback
table externally. The bootstrap does not contain the RP2040 adapter object.

The IPC object outlives both contract/service objects. The RP2040 adapter and
its callback context outlive the Core1 service.

## Bootstrap API

The bootstrap exposes t425_core_phy_bootstrap_init(),
t425_core_phy_bootstrap_core0() and t425_core_phy_bootstrap_poll_core1().

Initialization validates the Core1 callback table, initializes IPC, attaches
the Core0 contract, initializes the Core1 service and sets initialized only
after all steps succeed. Repeated initialization is rejected. No
deinitialization or recovery of a partially initialized object is provided.

## Host Test Strategy

The behavior test uses mock Core1 callbacks. It validates invalid
initialization, valid initialization, repeated initialization rejection,
Core0 access, Core0 TX and RX mailbox behavior, endpoint identity, Core1 TX
and RX forwarding, TX backpressure preservation, RX-full preservation,
stable IPC ownership and null/uninitialized access behavior.

The RX mock has separate configured and queried endpoint fields. Availability
queries never overwrite the configured endpoint. Wrong-endpoint availability
is false, wrong-endpoint RX get cannot consume the configured frame, and the
configured frame is consumed exactly once.

The existing IPC source includes pico/util/queue.h. The test therefore uses a
test-only queue compatibility header and implementation. No Pico hardware,
PIO, DMA, GPIO or IRQ code is compiled.

Pre-implementation review rejected proposal d9610bc17819 because its RX mock
overwrote the configured endpoint during availability checking. That was a
test-mock defect caught before execution, not an architectural project issue.
This corrected validation explicitly verifies logical endpoint preservation.

## Isolated Host Build Boundary

The isolated command compiles only the ownership test, the test-only queue
shim, the validated portable contract, the existing IPC implementation and
the new bootstrap implementation. The executable is
/tmp/t425_f_phy_64_ownership_test.

## Architectural Invariants

F_PHY-64 preserves platform independence of source/t425_core, separation of
endpoint_id, physical_link_id and t425_channel_select, no FIFO or rendezvous
state in the architectural CORE, TX acceptance meaning Link Service
acceptance only, existing channel WAIT/TAKE behavior, no architectural OUT
completion claim and no physical routing decision in the CORE.

## Physical Boundary Exclusions

F_PHY-64 does not implement Core1 launch, canonical CMake integration, PIO,
DMA, ACK/retry, GPIO mapping, physical routing, physical frame encoding,
architectural IN/OUT completion or rendezvous integration.

IMS physical documentation is not required for implementation of this
ownership/binding seam. Original IMS T425 documentation remains mandatory
before subsequent physical-driver decisions.

## Acceptance Criteria

Acceptance requires exact bootstrap state and API, explicit Core0 and Core1
ownership, deterministic initialization order, repeated-initialization
rejection, correct Core0 TX/RX mailbox behavior, endpoint-preserving Core1
TX/RX forwarding, TX backpressure preservation, RX-full preservation,
host-only validation with output under /tmp, no changes to the architectural
CORE, F_PHY-63 contract, IPC files, RP2040 physical driver or CMake, and the
exact output: F_PHY-64 ownership binding: PASS.
