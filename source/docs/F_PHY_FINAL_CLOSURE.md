# T425-SIM  FINAL PHY CLOSURE

BASELINE = 053d3002654d0c1b3de739db7b1b382268d193c3
FINAL HEAD before commit = 053d3002654d0c1b3de739db7b1b382268d193c3

## CLOSED STAGES
F_PHY-63, F_PHY-64, F_PHY-65, F_PHY-68, F_PHY-69, F_PHY-70, F_CORE-SYS-01, F_PHY-71, F_PHY-71C, F_PHY-71D, F_PHY-71E, F_PHY-71E3, F_CORE-IN-01.

## FINAL RESULT
FINAL_PHY_STATUS = PASS

## ARCHITECTURAL INVARIANTS
Four full-duplex physical links use two PIO state machines per link, eight total. Endpoint 0..3 corresponds directly to architectural Link0..3 only at the CORE-PHY seam. Physical ACK remains PHY-local, with one outstanding DATA byte per direction and outgoing ACK bypassing DATA wait. Portable CORE has no RP2040, PIO, DMA, or GPIO dependency.

## CORE-PHY BOUNDARY
RX admission is controlled by CORE architectural state. IPC publication is coherent and consumable one credit at a time. WAIT_CONTROL arbitration is global and deterministic. The external IN lifetime is explicit.

## PHYSICAL LINK BEHAVIOR
DATA capture is distinct from acceptance. A successful accepted DATA byte enables exactly one PHY-local ACK. F_PHY-69 outgoing DATA ACK gating remains unchanged.

## BOOTSTRAP / RX ADMISSION
Bootstrap LOAD grants one byte at a time. Multibyte admission is rearmed one byte at a time. After the final byte, N+1 remains unaccepted and unACKed until another architectural IN.

## EXTERNAL IN LIFETIME
The real path is PFIX 0 -> OPR 0x07 -> CROM[007] -> MIR microsequence -> SelectChannelReg -> CHANNEL 0x004 EnableChannelandEnableChannelStatus -> explicit Link Input classification -> external_in_begin(). At enable the observed state is SelectChannel=0x80000010, byte_count=4, buffer=0x80000200. Lifetime progresses 4 -> 3 -> 2 -> 1 -> 0.

## TEST EVIDENCE
Focused real OPR IN lifetime and multibyte tests pass. Canonical build and git diff check pass. CROM contents and MIR encoding remain unchanged; MIR CHANNEL width is 11 bits.

## KNOWN ITEMS OUTSIDE PHY SCOPE
Error / ErrorIn, Analyse / HaltOnError, EventReq / EventAck / EventWaiting, ProcSpeedSelect, and DisableIntRAM belong to future CORE work and are not PHY blockers. VM is not implemented in this closure; VM work starts after this checkpoint.
