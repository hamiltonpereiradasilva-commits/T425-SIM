# F_CORE-IN-01B — canonical IN path corrections

## Requirement

Preserve the CROM/MIR microsequencer and implement the audited eleven-bit
channel-control field without adding an opcode-specific instruction handler.

## Channel-control field

The canonical field remains Microword[74,11:9,7:1], extracted in the
existing bit order. Its API now returns uint16_t, preserving all 0x001
through 0x400 request bits.

Supported requests use existing canonical state:
- ChannelDataRegfromUbus updates t425_channel_data_reg from the existing CORE data register path.
- DataMemAddrfromSelectChannelReg copies t425_channel_select to t425_data_mem_addr.
- Reset/ready/run, ALT, and channel-status requests update their existing CORE fields.

The high/low architectural channel ACK requests remain blocked because the
new CORE has no canonical scheduler/rendezvous ACK state or executable
successful rendezvous transition. They are not mapped to physical ACK.

## Real OPR IN

The production path remains fetch -> microentry -> MIR -> microcycle.
The focused real PFIX 0 / OPR 0x07 probe reaches the canonical microsequence
but does not reach a successful channel ACK/take transition that can start
t425_core_external_in_begin(). No legacy code or alternate opcode handler
was added.

## External lifetime

The pre-existing F_PHY-71E lifetime state and portable RX consumption remain
unchanged. A PASS requires the real OPR IN rendezvous to supply the
architectural A/B/C operands at the missing successful transition; that
canonical state is not currently exposed.

## Provenance

No CROM contents, MIR encoding, physical ACK protocol, PHY, PIO, DMA, or
channel-select physical-link mapping was changed.

## F_CORE-IN-01E closure status

The speculative ChannelDataRegfromUbus-to-DataReg mapping was removed.
The canonical datapath does not expose an explicit U-bus value at the
channel-control boundary, so 0x400 remains recognized but unexecuted.

The canonical scheduler exposes FPtr/BPtr queues and current_wdesc, but
does not expose CHAN0REQ/CHAN1REQ pending request state or the successful
microcoded rendezvous transition required to execute 0x200/0x100. The
real OPR 0x07 path therefore cannot yet establish external_in_begin()
without inventing state or an opcode/MPC shortcut.


## F_CORE-IN-01 final closure

STATUS = CLOSED / PASS

Real path: PFIX 0 -> OPR 0x07 -> CROM[007] -> real microsequence -> SelectChannelReg -> CHANNEL 0x004 EnableChannelandEnableChannelStatus -> external Link Input classification -> external_in_begin().

Observed at enable: SelectChannel = 0x80000010, byte_count = 4, buffer = 0x80000200. External Link Input admission starts from the semantic channel-enable operation, never from physical ACK or opcode dispatch.

Multibyte lifetime is 4 -> 3 -> 2 -> 1 -> 0; N+1 remains blocked until another architectural IN. Physical ACK remains PHY-local. CROM and MIR encoding are unchanged; MIR CHANNEL width is 11 bits.
