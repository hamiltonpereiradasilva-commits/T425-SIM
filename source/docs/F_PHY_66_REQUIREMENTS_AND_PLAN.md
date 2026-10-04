# F_PHY-66 Requirements and Plan

## Status and objective

F_PHY-63, F_PHY-64 and F_PHY-65 are CLOSED and remain unchanged.

F_PHY-66 objective:

IMS T425 PHYSICAL-CONTRACT EVIDENCE AND OWNERSHIP GATE

This document closes the documentary gate. It does not implement physical-driver
behavior.

## Authority hierarchy

LEVEL 1 — AUTHORITATIVE

Original IMS/INMOS/SGS-THOMSON documentation.

LEVEL 2 — PROJECT TRACEABILITY

T425-SIM traceability, requirements and audit documents.

LEVEL 3 — CURRENT IMPLEMENTATION

The canonical t425_link, PIO, adapter and build sources.

Level 3 must be checked against Level 1. Level 3 must never prove Level-1
semantics. Legacy UART/RS232 material is not authoritative.

## Primary documents

T_425.pdf

- IMS T425
- SGS-THOMSON
- February 1996
- 70 pages

tdata1st.pdf

- Transputer Databook, First Edition 1988
- INMOS Limited
- document 72-TRN-203-00
- 497 pages

Primary references used:

- T_425.pdf, printed page 5.
- T_425.pdf, printed pages 9-11.
- T_425.pdf, printed pages 42-47.
- tdata1st.pdf, printed pages approximately 160-166.

## Authoritative signal contract

| Signal | Direction at T425 | Purpose and scope | Idle/reset behavior |
|---|---|---|---|
| LinkIn0..3 | Input | Four serial input channels, one per logical link | Link inputs are held LOW during reset |
| LinkOut0..3 | Output | Four serial output channels, one per logical link | Link outputs are quiescent LOW and forced LOW during reset |
| LinkSpecial | Input | Selects non-standard link speed | Shared speed-selection input |
| Link0Special | Input | Selects special speed for Link0 | Applies to Link0 speed selection |
| Link123Special | Input | Selects special speed for Links1, 2 and 3 | Applies to Links1-3 speed selection |
| Reset | Input | Initializes the transputer and starts bootstrap on its falling edge | Link outputs LOW; link inputs held LOW during reset |
| Analyse | Input | Halts at a descheduling point while preserving required machine state | Outstanding input transfers continue; buffered output may finish |
| Error | Output | OR of internal Error flag and ErrorIn | Global system-service signal |
| ErrorIn | Input | Error daisy-chain input | Does not directly alter processor status |

References: T_425.pdf printed pp. 5 and 9-11.

## Authoritative link framing

Reference: T_425.pdf printed p. 42, section 7 and Figure 7.1.

- Idle level: LOW.
- Data packet: HIGH START, HIGH data control/type bit, eight data bits,
  LSB first, LOW STOP.
- ACK packet: HIGH START followed by LOW control/type bit.
- Data and ACK use the same physical link signaling.
- The control/type bit distinguishes data from ACK.

## ACK semantics

Reference: T_425.pdf printed p. 42.

- Every transmitted byte is acknowledged on the input of the same link.
- ACK means that the receiver accepted the byte and can receive another byte.
- The sender waits for the ACK after transmitting a data byte.
- The sending process is rescheduled only after the ACK for the final message
  byte.
- An ACK may be sent before the complete data packet has been received.
- ACK is therefore per byte/token and supports overlapped acknowledge.

## Timing, speed and electrical contract

Protocol requirements:

- Supported speeds: 5, 10 and 20 Mbit/s.
- Standard speed: 10 Mbit/s.
- Links are asynchronous to ClockIn and ProcClockOut.
- Independently clocked systems may communicate when clocks are nominally
  identical and within specification.
- A 100-ohm transmission line should have total delay below 0.4 bit time.

Electrical requirements:

- TTL-compatible interface.
- VDD operating range: 4.75-5.25 V.
- VIH minimum: 2.0 V.
- VIL maximum: 0.8 V.
- LinkOut rise/fall: 20 ns / 10 ns.
- LinkIn rise/fall: 20 ns / 20 ns.
- LinkIn capacitance: 7 pF.
- LinkOut load capacitance: 50 pF.
- General load capacitance: 60 pF excluding LinkOut load.
- 100-ohm transmission-line guidance uses a 56-ohm series resistor.

These values are from T_425.pdf printed pp. 42-47, especially Tables 7.1,
7.2, 8.2 and 8.3.

The categories remain distinct:

- PROTOCOL REQUIREMENT: framing, ACK, overlap, supported speeds and
  asynchronous link behavior.
- ELECTRICAL REQUIREMENT: TTL levels, voltage, edge, capacitance, line delay
  and resistor guidance.
- RP2040 IMPLEMENTATION CHOICE: PIO, DMA, IRQ, GPIO numbering, buffering and
  resource allocation.

## Reset, Analyse and Error

Reset is global. Its falling edge initializes the device; outputs are forced LOW
during reset and inputs are held LOW.

Analyse is global. The processor halts at a permitted descheduling point.
Outstanding input transfers continue, while output transfers do not fetch new
data but may transmit bytes already buffered.

Error is the OR of the internal Error flag and ErrorIn. ErrorIn supports a
daisy-chain and does not directly change processor status. Links finish
outstanding transfers when the processor halts through HaltOnError.

Reference: T_425.pdf printed pp. 9-11.

## Level-1 to Level-3 traceability matrix

| IMS requirement | Primary reference | Current implementation | Classification | Required future action | Category | Blocks physical activation |
|---|---|---|---|---|---|---|
| Four LinkIn/LinkOut channels | T_425.pdf p. 5, p. 42 | Four t425_link_t pointers and four-link initializer | PARTIAL | Complete resource ownership and validation | RESOURCE OWNERSHIP | Yes |
| Quiescent LOW output | T_425.pdf p. 42 | No explicit initial LOW operation in TX setup | PARTIAL | Establish and validate initial output state | PHYSICAL SEMANTICS | Yes |
| HIGH START | T_425.pdf p. 42 | build_data_word/build_ack_word set START HIGH | MATCH at encoding level | Hardware timing validation | PHYSICAL SEMANTICS | Yes |
| Data type bit HIGH | T_425.pdf p. 42 | Data word sets type bit HIGH | MATCH at encoding level | Runtime validation | PHYSICAL SEMANTICS | Yes |
| ACK type bit LOW | T_425.pdf p. 42 | ACK word sets type bit LOW | MATCH at encoding level | Complete ACK path | PHYSICAL SEMANTICS | Yes |
| Eight payload bits | T_425.pdf p. 42 | Payload shifted into eight serial positions | MATCH at encoding level | Validate complete path | PHYSICAL SEMANTICS | Yes |
| LSB first | T_425.pdf p. 42 | Shift-right PIO configuration | PARTIAL | Validate generated waveform | PHYSICAL SEMANTICS | Yes |
| LOW STOP | T_425.pdf p. 42 | TX word leaves STOP LOW; RX checks STOP | PARTIAL | Validate output state and malformed input handling | PHYSICAL SEMANTICS | Yes |
| Per-byte ACK | T_425.pdf p. 42 | tx_waiting_ack declared but not enforced | MISMATCH | Implement documented ACK state | PHYSICAL SEMANTICS | Yes |
| Overlapped ACK | T_425.pdf p. 42 | Adapter attempts IRQ-based ACK generation | NOT OPERATIONAL | Resolve event and scheduling path | PHYSICAL SEMANTICS | Yes |
| 5/10/20 Mbit/s | T_425.pdf p. 42, Table 7.1 | Speed enum and divisors exist | PARTIAL | Validate speed selection | PHYSICAL/ELECTRICAL | Yes |
| Asynchronous links | T_425.pdf p. 42 | PIO uses clk_sys-derived divisors | PARTIAL | Validate tolerance and phase | PHYSICAL | Yes |
| Edge, skew and load limits | T_425.pdf p. 43, Table 7.2 | No waveform validation | NOT YET IMPLEMENTED | Hardware/logic-analyzer validation | ELECTRICAL/HARDWARE | Yes |
| TTL voltage behavior | T_425.pdf pp. 45-47 | No RP2040 level-translation proof | NOT YET IMPLEMENTED | Board and interface validation | ELECTRICAL/HARDWARE | Yes |
| LinkSpecial speed controls | T_425.pdf p. 5, p. 42 | Adapter decodes GPIO 20/21/22 | PARTIAL | Confirm board wiring | HARDWARE | Yes |
| Reset link behavior | T_425.pdf p. 9 | No complete reset ownership | NOT YET IMPLEMENTED | Platform reset sequencing | PLATFORM | Yes |
| Analyse behavior | T_425.pdf pp. 9-10 | Not implemented in adapter | NOT APPLICABLE AT THIS LAYER | Defer to system-service work | ARCHITECTURAL | No for this PHY gate |
| Error/ErrorIn behavior | T_425.pdf pp. 5, 11 | Not implemented in adapter | NOT APPLICABLE AT THIS LAYER | Defer to system-service work | PLATFORM | No for this PHY gate |
| TX PIO serialization | T_425.pdf p. 42 | t425_link.pio serializes count and sequence | PARTIAL | Validate timing, idle state and ACK gating | PHYSICAL | Yes |
| RX start detection | T_425.pdf p. 42 | wait HIGH then immediate sample | PARTIAL | Validate sampling phase | PHYSICAL | Yes |
| RX framing reconstruction | T_425.pdf p. 42 | 11 samples and explicit push | PARTIAL | Validate framing and malformed tokens | PHYSICAL | Yes |
| RX ACK recognition | T_425.pdf p. 42 | Adapter recognizes ACK word | PARTIAL | Connect to working ACK state path | PHYSICAL | Yes |
| ACK IRQ event | IMS specifies ACK semantics, not IRQ | RX PIO has no irq instruction; adapter expects PIO IRQ | MISMATCH | Resolve RP2040 event mechanism | API/RESOURCE/PHYSICAL | Yes |
| TX DMA | IMS does not mandate DMA | Persistent TX word and one-word transfer | NOT APPLICABLE AT THIS LAYER | Validate as implementation choice | RESOURCE OWNERSHIP | Yes |
| RX DMA | IMS does not mandate DMA | Configured but not started, rearmed or consumed | NOT OPERATIONAL | Choose coherent buffering design | RESOURCE OWNERSHIP | Yes |
| GPIO numbering | IMS specifies logical link names only | L2/L3 use 17/16 and 19/18; comments mention 4/5 and 6/7 | NOT APPLICABLE AT THIS LAYER | Resolve from hardware evidence | ELECTRICAL/HARDWARE | Yes |
| Four-link SM layout | IMS requires four logical links, not SM numbers | init_four_links maps PIO0/PIO1 SM pairs | PARTIAL | Add claiming and collision ownership | RESOURCE OWNERSHIP | Yes |
| PIO program lifetime | IMS does not define PIO | pio_add_program without removal/lifetime owner | MISSING | Define owner and cleanup | RESOURCE OWNERSHIP | Yes |
| Partial-init cleanup | IMS does not define cleanup | t425_link_dma_deinit declared but absent | MISSING | Define rollback and deinit | RESOURCE OWNERSHIP | Yes |

## TX PIO audit

t425_link.pio serializes the count and a right-shifted sequence. At the
encoding level, current construction matches IMS data and ACK framing.

The implementation remains PARTIAL and is not ready for physical activation
because:

- idle LOW is not explicitly guaranteed;
- ACK gating is not implemented;
- t425_link_send_frame and t425_link_send_ack have bool/void declaration
  mismatches;
- clock timing has not been physically validated;
- state-machine claiming is absent;
- PIO program lifetime is not owned.

## RX PIO audit

t425_link_rx.pio:

- waits for HIGH START;
- samples START, TYPE, D0-D7 and STOP;
- pushes a complete sampled word;
- supports adapter-side ACK recognition.

The RX PIO does NOT generate the event expected by the adapter ACK IRQ path.
There is no irq instruction in the RX PIO, while the adapter's
service_ack_flags path expects a PIO IRQ.

The RX result is therefore PARTIAL with an explicit IRQ-path mismatch.
Sampling phase, framing reconstruction and ACK recognition remain subject to
physical validation.

## DMA findings

IMS specifies external protocol behavior, not RP2040 DMA.

TX DMA has a persistent TX word, but ACK gating, backpressure and
serialization are not proven.

RX DMA is configured but not operationally started, rearmed or consumed.
Related ring/full/consumed state is not maintained operationally. RX currently
reads the PIO FIFO directly.

## Known blockers

| Blocker | Status | Classification | IMS evidence |
|---|---|---|---|
| t425_link_send_frame signature mismatch | Present | BUILD/API | Not required |
| t425_link_send_ack signature mismatch | Present | BUILD/API | Not required |
| Missing t425_link_dma_deinit | Present | BUILD/LIFECYCLE | Not required |
| Missing t425_link_note_ack_received | Present | BUILD/PROTOCOL STATE | ACK meaning available from IMS |
| RX DMA lifecycle | Incomplete | RUNTIME/RESOURCE | Not required |
| ACK IRQ producer/consumer mismatch | Present | RUNTIME/PHYSICAL | Required and now available for ACK semantics |
| PIO state-machine claiming | Missing | RUNTIME/RESOURCE | Not required |
| PIO program lifetime | Missing | RUNTIME/RESOURCE | Not required |
| GPIO map discrepancy | Present | ELECTRICAL/HARDWARE | IMS does not define RP2040 numbers |
| Four-link ownership | Incomplete | RUNTIME/RESOURCE | Logical four-link requirement available |
| Partial-init cleanup | Incomplete | RUNTIME/RESOURCE | Not required |
| t425_core_engine | Undefined | BUILD/API | IMS cannot establish a CMake target |

## GPIO conclusion

IMS defines logical Link0..3 identity, but does not define RP2040 GPIO numbers.

Current implementation:

- L0 TX/RX = 0/1
- L1 TX/RX = 2/3
- L2 TX/RX = 17/16
- L3 TX/RX = 19/18

Comments mention:

- L0 = 0/1
- L1 = 2/3
- L2 = 4/5
- L3 = 6/7

F_PHY-66 selects neither mapping. Board schematic, wiring and hardware
evidence remain required.

## Platform ownership

A future platform/Core1 owner should own:

- t425_core_phy_rp2040_t;
- four t425_link_t instances;
- RX buffers;
- route table;
- PIO program lifetime;
- TX/RX state-machine reservations;
- DMA channels;
- IRQ ownership;
- GPIO configuration;
- cleanup and rollback;
- Core1 entrypoint;
- physical service loop.

The neutral F_PHY-65 runtime remains RP2040-independent.

Ownership can be defined now. Resource acquisition must wait for API/lifecycle
consistency and hardware evidence.

## Core1 decision

CORE1 LAUNCH AFTER F_PHY-66: STILL DEFER

Prerequisites:

- complete physical-resource owner;
- consistent link APIs;
- working ACK event production;
- coherent RX buffering policy;
- PIO state-machine and program ownership;
- partial-init cleanup;
- GPIO confirmation;
- level-translation confirmation;
- startup readiness;
- shutdown/reset rules.

## t425_core_engine

UNRESOLVED — DO NOT ALIAS OR FAKE

IMS documentation cannot establish a CMake target.

## F_PHY-66 documentary conclusion

IMS PHYSICAL CONTRACT EVIDENCE SUFFICIENT — F_PHY-66 DOCUMENT GATE CAN CLOSE

## Recommended next milestone

F_PHY-67 — RP2040 PHY API and lifecycle consistency gate

Its single responsibility is to make the existing physical adapter internally
consistent and statically linkable without changing physical semantics.

F_PHY-67 is not implemented by this document proposal.

Deferred:

- PIO protocol changes;
- RX sampling redesign;
- DMA architecture redesign;
- GPIO assignment;
- electrical level shifting;
- ACK/retry policy changes;
- physical Link0..3 activation;
- Core1 launch;
- architectural CORE changes;
- VM integration.

## Architectural invariants

- The architectural CORE remains RP2040-independent.
- t425_channel_select remains architectural, not physical.
- endpoint_id remains service-level and distinct from physical link identity.
- No architectural IN/OUT completion semantics are invented.
- No UART/RS232 physical protocol is reused.
- F_PHY-63, F_PHY-64 and F_PHY-65 remain unchanged.

## GO/NO-GO

GO:

- F_PHY-66 documentation closure.
- Planning F_PHY-67 after F_PHY-66 is formally closed.

NO-GO:

- physical implementation during this proposal;
- PIO, DMA, GPIO, ACK, Core1 or architectural CORE changes.
