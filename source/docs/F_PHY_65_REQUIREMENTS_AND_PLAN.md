# F_PHY-65 Requirements and Plan

## Baseline
F_PHY-63 is CLOSED: the portable CORE-to-Link-Service callback contract is
implemented and behaviorally validated.
F_PHY-64 is CLOSED: Core0-to-Core1 IPC/mailbox ownership and bounded service
bootstrap are implemented and behaviorally validated.

## Refined scope
F_PHY-65 adds only nonphysical build ownership and a neutral runtime owner.
There is no physical adapter startup, Core1 launch, architectural CORE
attachment, IN/OUT completion, PIO, DMA, GPIO, IRQ, ACK/retry behavior,
physical routing, frame encoding, or legacy-runtime reuse.
The runtime owner is under source/t425_runtime/ because it depends only on
service/bootstrap and portable contract types. It does not own
t425_core_phy_rp2040_t or physical-link objects. The wrapper is justified by
stable future runtime lifetime and isolation from the current adapter path.

## Ownership and build
The runtime owns exactly one t425_core_phy_bootstrap_t and initialized state.
It accepts external Core1 callback operations, exposes Core0, and performs one
bounded Core1 poll. Core1 launch and physical initialization are DEFERRED.
The t425_core_phy_contract target contains only t425_core_phy.c.
The t425_core_phy_service target contains only existing IPC/bootstrap sources
and links the contract. The t425_runtime target contains only the neutral
owner and links contract/service. Root CMake adds only t425_core/phy and
t425_runtime; it must not add source/t425_phy/rp2040.
The undefined t425_core_engine dependency remains confined to the separate
physical target. F_PHY-65 must not invent, alias, or fake it.

## Validation and exclusions
Host validation uses strict C11, the F_PHY-64 queue shim, mock Core1
callbacks, and no physical sources. It verifies invalid/valid initialization,
stable ownership, repeated initialization rejection, Core0 TX, Core1 RX,
endpoint preservation, TX backpressure, RX-full preservation, and bounded
polling. Later RP2040 compile-only validation is deferred and cannot prove
INMOS compatibility, PIO timing, ACK behavior, DMA runtime behavior, or
interoperability.
The architectural CORE remains RP2040-independent. endpoint_id is distinct
from physical_link_id. t425_channel_select remains architectural. No FIFO or
rendezvous state is added to the portable contract. No architectural OUT
completion is claimed. No legacy runtime is reused. Physical-driver blockers
remain deferred.

IMS PHYSICAL DOCUMENTATION NOT REQUIRED FOR F_PHY-65
RUNTIME/BUILD OWNERSHIP WORK.

IMS PHYSICAL DOCUMENTATION REMAINS MANDATORY BEFORE
PHYSICAL DRIVER IMPLEMENTATION.

## Acceptance
F_PHY-65 requires unchanged F_PHY-63/F_PHY-64 behavior, explicit neutral
targets, root reachability without the blocked physical target, explicit
runtime lifetime, passing host validation, no physical semantics, no Core1
launch, no architectural IN/OUT completion, no legacy runtime reuse, and this
document. GO is limited to this nonphysical boundary; physical startup,
physical semantics, Core1 launch, architectural IN/OUT, and VM integration
remain NO-GO.
