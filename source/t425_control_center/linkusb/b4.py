import struct
B4_LENGTH = 56
MAGIC = b"F271"
class B4Error(ValueError):
    pass

def decode_b4(raw, expected_link=None):
    raw = bytes(raw)
    if len(raw) != B4_LENGTH:
        raise B4Error("B4 length %d, expected 56" % len(raw))
    if raw[:4] != MAGIC or raw[4] != 1:
        raise B4Error("B4 magic/version mismatch")
    link = raw[5]
    if link not in (0, 1) or (expected_link is not None and link != expected_link):
        raise B4Error("B4 link index mismatch")
    flags = raw[6]
    u32 = lambda off: struct.unpack_from("<I", raw, off)[0]
    u16 = lambda off: struct.unpack_from("<H", raw, off)[0]
    return {
        "link": link, "started": bool(flags & 1), "tx_waiting_ack": bool(flags & 2),
        "data_pending": bool(flags & 4), "physical_ack_pending": bool(flags & 8),
        "usb_out_bytes": u32(8), "host_tx_enqueued_bytes": u32(12),
        "phy_tx_submitted_frames": u32(16), "ack_rx_frames": u32(20),
        "phy_rx_data_frames": u32(24), "ack_tx_submitted_frames": u32(28),
        "host_rx_enqueued_bytes": u32(32), "usb_in_accepted_bytes": u32(36),
        "host_tx_queue_depth": u16(40), "host_rx_queue_depth": u16(42),
        "malformed_words": u32(44), "tx_ready": bool(u32(48)),
        "physical_pin_capture": False,
    }
