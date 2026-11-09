from dataclasses import dataclass
from hashlib import sha256
from pathlib import Path

HELLO_SHA256 = "36af0ff47e059603b1fd18ca832c668ab519ea7cfa3c9bd8db364f8f8a1b661a"
HELLO_SIZE = 3267

@dataclass(frozen=True)
class BtlInfo:
    path: str
    name: str
    size: int
    sha256: str
    first_byte: int
    expected_hello: bool

def inspect_btl(path):
    p = Path(path).expanduser().resolve()
    if not p.is_file():
        raise ValueError("O arquivo BTL selecionado não existe")
    digest = sha256()
    size = 0
    first = None
    with p.open("rb") as f:
        while True:
            block = f.read(1024 * 1024)
            if not block:
                break
            if first is None:
                first = block[0]
            size += len(block)
            digest.update(block)
    if first is None:
        raise ValueError("O arquivo BTL está vazio")
    hexsum = digest.hexdigest()
    return BtlInfo(str(p), p.name, size, hexsum, first,
                   size == HELLO_SIZE and hexsum == HELLO_SHA256)
