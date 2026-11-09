import json
import serial
from serial.tools import list_ports

T425_VID, T425_PID = 0x2E8A, 0x000A
T425_SERIAL = "50444672C0A3541C"

def find_t425_port():
    for port in list_ports.comports():
        if port.vid == T425_VID and port.pid == T425_PID and port.serial_number == T425_SERIAL:
            return port.device
    return None

def query_f271(port=None, timeout=1.5):
    port = port or find_t425_port()
    if not port:
        raise RuntimeError("T425-SIM CDC não encontrado pelo serial esperado")
    with serial.Serial(port, 115200, timeout=timeout, write_timeout=timeout,
                       dsrdtr=False, rtscts=False) as ser:
        ser.dtr = True
        ser.rts = True
        ser.reset_input_buffer()
        ser.write(b"F271?\n")
        ser.flush()
        raw = ser.readline()
    if not raw:
        raise TimeoutError("F271? sem resposta CDC")
    try:
        return json.loads(raw.decode("utf-8"))
    except Exception as exc:
        raise RuntimeError("Resposta F271? inválida: %r" % raw) from exc
