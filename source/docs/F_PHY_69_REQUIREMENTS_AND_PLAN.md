# F_PHY-69 Requirements and Plan

## Scope

F_PHY-69 implements per-byte physical TX ACK gating in the canonical RP2040 PHY. It preserves the separation between PHYSICAL_LINK_ACK and LOGICAL_CHANNEL_HANDSHAKE.

## IMS invariant

After a DATA byte is committed, the sender waits for its physical ACK before committing another DATA byte on that same physical link. At most one DATA byte is unacknowledged per direction and link.

## State machine

Each link has PHY-local READY and WAIT_ACK states represented by tx_waiting_ack. Initialization, stop, and deinitialization leave the state READY. A successful DATA submission transitions READY to WAIT_ACK. An incoming physical ACK through the existing RX FIFO and pump_rx path clears the state.

## ACK bypass

tx_waiting_ack gates DATA only. Outgoing PHYSICAL_LINK_ACK generated for received DATA remains eligible while the link waits for an ACK for its own DATA.

## Independence and isolation

The state is per Link0..Link3. No global TX serialization is introduced. No CORE, scheduler, channel rendezvous, WAIT/TAKE, t425_channel_select, RX DMA, retry, timeout, retransmission, or Core1 behavior changes.

## Acceptance

The dedicated host test proves initial eligibility, arming, same-link blocking, ACK release, next DATA eligibility, per-link independence, ACK bypass, and lifecycle clearing. Existing F_PHY-67 and F_PHY-68 tests, canonical configure/build, PIO provenance, and canonical-tree isolation must remain passing.
