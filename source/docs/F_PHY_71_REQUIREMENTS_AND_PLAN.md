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
