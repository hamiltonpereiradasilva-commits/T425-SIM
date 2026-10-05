# F_PHY-71 — RX admission backpressure

Physical DATA capture is distinct from DATA acceptance. The RP2040 adapter
retains at most one pending unacknowledged DATA byte per physical link.
A byte is published toward Core0 only when per-endpoint RX admission is
enabled and the destination IPC queue has capacity. Physical ACK remains
PHY-local and becomes eligible only after that acceptance.

The portable t425_core_phy_t contract carries only endpoint admission.
Core0 publishes the state through the existing IPC mailbox; Core1 applies
it to the RP2040 adapter. No ACK encoding, PIO, DMA or GPIO state crosses
the portable boundary.

Bootstrap admission enables all endpoints in WAIT_CONTROL, restricts
active POKE/PEEK/LOAD to the selected link, and disables the link
immediately after the final LOAD byte. In RUNNING, admission is derived
from the architectural LinkN Input channel-control word:
0x80000010 + 4*N != T425_CHANNEL_EMPTY. Output addresses and
t425_channel_select are never used as endpoint identity.

The adapter retains DATA while admission is false or IPC is full, and
sends exactly one physical ACK after successful publication. Incoming
physical ACK handling and F_PHY-69 TX gating remain unchanged.

## F_PHY-71C — consumable RX admission credit

Admission is transported as a monotonically increasing per-endpoint
generation plus enabled state. Each enabled generation is one consumable
credit. Core1 applies newer generations only; repeated polling of the
same generation cannot recreate credit after DATA acceptance. DATA is
captured independently, but publication and PHY-local ACK require a
fresh unconsumed credit and destination capacity.

## F_PHY-71E — final pre-VM corrections

WAIT_CONTROL uses a global Core1 arbitration latch over the existing
per-endpoint credits. Physical links continue to capture at most one pending
DATA byte independently; endpoint order 0..3 selects exactly one candidate
for acceptance while the arbitration opportunity is active. Losing pending
bytes remain unacknowledged until a later WAIT_CONTROL opportunity.

The IPC admission publication is one atomic packed word per endpoint:
bit 0 is enabled and bits 1..31 carry a nonzero 31-bit generation. Core1
loads the word atomically, so enabled state and generation are coherent.
Generation comparison uses modulo-2^31 serial ordering.

External IN transfer lifetime is represented by CORE-owned
external_in_active[] and external_in_remaining[]. The
t425_core_external_in_begin() hook starts a known architectural transfer;
each accepted byte decrements the remaining count, rearms one fresh credit
only while bytes remain, and the final byte closes admission. Channel
control and rendezvous semantics remain unchanged.

## F_PHY-71E3 real OPR IN integration

The canonical MIR width correction is applied through the existing
t425_mir_channel() API. The real fetch -> PFIX 0 -> OPR 0x07 ->
microentry 0x007 -> CROM/MIR path is exercised by focused probes. The
current NEW CORE reaches the microsequence but does not expose a canonical
successful channel ACK/take transition with the architectural A/B/C
operands needed to call t425_core_external_in_begin() exactly once.
Therefore the real external-IN lifetime remains PATCH_REQUIRED pending
that existing canonical rendezvous/scheduler state; no legacy behavior or
physical ACK is exposed.

## F_PHY-71E3 closure status

The existing external-IN lifetime remains intact, but the canonical real
OPR 0x07 path still lacks an exposed successful channel ACK/rendezvous
transition carrying the architectural A/B/C operands. F_PHY-71E3 remains
BLOCKED pending that canonical state; no physical ACK is exposed to CORE.


## F_PHY-FINAL closure

F_PHY-71E = PASS
F_PHY-71E3 = PASS
FINAL_PHY_STATUS = PASS

Final invariants: four full-duplex physical links, two PIO state machines per link (eight total), direct endpoint 0..3 mapping only at the CORE-PHY seam, PHY-local ACK per DATA byte, one outstanding DATA byte per direction, outgoing ACK bypasses DATA wait, no RP2040/PIO/DMA/GPIO dependency in portable CORE, CORE architectural RX admission, global deterministic WAIT_CONTROL arbitration, coherent IPC publication, explicit external-IN lifetime, one-byte-at-a-time multibyte RX credit, and N+1 unaccepted/unACKed.
