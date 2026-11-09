import usb.core
import usb.util

VID, PID = 0x1209, 0x4250
INTERFACE = 0
EP_OUT, EP_IN = 0x02, 0x86

class LinkUsbDiagnosticSession:
    """Read-only B4/B2 session. Never reads Bulk IN or writes Bulk OUT."""
    def __init__(self, device=None):
        self.device = device
        self.claimed = False

    def connect(self):
        if self.device is not None:
            return
        dev = usb.core.find(idVendor=VID, idProduct=PID)
        if dev is None:
            raise RuntimeError("LINKUSB 1209:4250 não encontrado")
        try:
            try:
                cfg = dev.get_active_configuration()
            except usb.core.USBError:
                cfg = None
            if cfg is None:
                dev.set_configuration(1)
                cfg = dev.get_active_configuration()
            if cfg.bConfigurationValue != 1:
                raise RuntimeError("LINKUSB não está na configuração USB 1")
            intf = usb.util.find_descriptor(cfg, bInterfaceNumber=0, bAlternateSetting=0)
            if intf is None:
                raise RuntimeError("Interface 0/alt 0 não encontrada")
            eps = {e.bEndpointAddress: e for e in intf}
            if EP_OUT not in eps or EP_IN not in eps:
                raise RuntimeError("Endpoints Bulk do Link0 ausentes")
            usb.util.claim_interface(dev, INTERFACE)
            self.claimed = True
            self.device = dev
        except Exception:
            usb.util.dispose_resources(dev)
            raise

    def b4(self, link=0):
        if self.device is None:
            raise RuntimeError("LINKUSB desconectado")
        from .b4 import decode_b4
        raw = self.device.ctrl_transfer(0xC0, 0xB4, 0, int(link), 56, timeout=1000)
        return decode_b4(raw, int(link))

    def b2(self):
        if self.device is None:
            raise RuntimeError("LINKUSB desconectado")
        return bytes(self.device.ctrl_transfer(0xC0, 0xB2, 0, 0, 2, timeout=1000))

    def close(self):
        dev, self.device = self.device, None
        if dev is not None:
            try:
                if self.claimed:
                    usb.util.release_interface(dev, INTERFACE)
            finally:
                self.claimed = False
                usb.util.dispose_resources(dev)
