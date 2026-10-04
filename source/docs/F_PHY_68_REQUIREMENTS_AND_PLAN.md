# F_PHY-68 Requirements and Plan

## Closed baseline and objective

F_PHY-63, F_PHY-64, F_PHY-65, F_PHY-66 and F_PHY-67 are CLOSED. F_PHY-68
implements only the already-closed physical acknowledgement event boundary:
timely outgoing acknowledgement for received DATA while preserving the
authoritative complete-token FIFO path for DATA and incoming PHYSICAL_LINK_ACK.

PHYSICAL_LINK_ACK is independent from LOGICAL_CHANNEL_HANDSHAKE. No physical
event enters CORE, scheduler, WAIT/TAKE, channel rendezvous or architectural
IN/OUT completion.

## IMS Level-1 evidence

Primary sources are T_425.pdf (IMS T425, SGS-THOMSON, February 1996) and
tdata1st.pdf (Transputer Databook, First Edition 1988, document 72-TRN-203-00).
Established references are T_425.pdf printed p. 42, pp. 42-47 and pp. 9-11,
with supporting Databook material approximately printed pp. 160-166. IMS
framing is HIGH START, HIGH DATA TYPE, eight LSB-first data bits and LOW STOP;
a physical ACK is HIGH START with LOW TYPE, per-byte acceptance, and may
overlap remaining token/message reception. IMS leaves internal IRQ/FIFO/DMA
transport implementation-defined.

## Closed event architecture

RX PIO samples START and TYPE. Only TYPE=DATA emits irq nowait 0 rel, after
TYPE classification. The PIO then samples D0 through STOP and pushes one
complete token. TYPE=ACK emits no DATA event and is pushed identically. Thus
the RX FIFO remains authoritative. pump_rx() identifies an incoming ACK,
increments physical_ack_count[] and calls t425_link_note_ack_received() for
PHY-local state only. service_ack_flags() consumes the DATA-qualified event
and submits an outgoing PHYSICAL_LINK_ACK on the same link.

The IRQ is RECEIVED_DATA_REQUIRES_PHYSICAL_ACK, never RECEIVED_PHYSICAL_ACK and
never a logical or architectural acknowledgement.

## Source of truth and IRQ mapping

t425_link_rx.pio is the editable source of truth. CMake generates the header
with pico_generate_pio_header(); the generated header is not hand-patched.
Relative irq nowait 0 rel targets the current RX SM flag: SM1 maps to flag 1
and SM3 to flag 3. PIO0 maps L0 TX0/RX1 and L1 TX2/RX3; PIO1 maps L2 TX0/RX1
and L3 TX2/RX3. Each link retains independent FIFO, IRQ and pending state.

## Lifecycle and event coalescing

Stop disables relevant IRQ sources and clears physical_ack_pending[] and
physical_ack_count[]. One valid outstanding DATA byte per physical direction
makes one pending outgoing ACK state per link sufficient; independent links
remain concurrent. No RX DMA is activated. TX ACK gating, retry, timeout,
retransmission, PIO ownership, GPIO/electrical changes, Core1 and
t425_core_engine remain deferred.

## Validation

The deterministic test regenerates the RX header twice with picoasm and
compares outputs. It verifies DATA-only IRQ placement after TYPE, complete
FIFO push, incoming ACK decoding, outgoing service_ack_flags() coupling,
four-link-independent adapter structure, lifecycle clearing, and exact output:

F_PHY-68 DATA-qualified IRQ source: PASS

Hardware waveform timing, overlap, concurrent four-link operation, voltage and
signal integrity remain separate validation.

## Acceptance and exclusions

No CORE, runtime, IPC, CMake, GPIO, RX DMA, TX gating, scheduler or channel
changes are included. Hardware validation remains required before physical
interoperability. F_PHY-69 has not begun.
