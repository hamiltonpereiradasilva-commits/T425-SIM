# T425-SIM v2.20 — Transport + Router

A v2.20 integra o pacote endereçado com o roteador.

## Wire format

```text
MAGIC | VER | DEST | SOURCE | TTL | SEQ | LEN | RSV | PAYLOAD | CRC16
```

O `Transport` serializa/desserializa e valida CRC. O `Router` decide o próximo enlace.

## Fluxo

```text
Application
    ↓
Packet
    ↓
Transport encode
    ↓
Reliable Link
    ↓
PIO
    ↓
[ physical link ]
    ↓
PIO
    ↓
Reliable Link
    ↓
Transport decode
    ↓
Router
    ├── local → Application
    └── remote → próximo link
```

## Teste lógico E2E

A mensagem:

```text
SOURCE=0 DEST=3 PAYLOAD="HELLO"
```

é encaminhada:

```text
P00 → P01 → P02 → P03
```

com TTL:

```text
8 → 7 → 6 → 5
```

O teste verifica também corrupção de payload: uma alteração de byte deve invalidar o CRC.

A v2.20 é ainda uma integração de software. A confirmação elétrica continua sendo realizada no hardware quando os quatro Pico estiverem conectados.
