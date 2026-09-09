# T425-SIM v2.22 — Application → Router → Reliable Link

A v2.22 conecta a intenção da aplicação ao Router e ao Reliable Link.

## Fluxo real no software

```text
SEND 3 HELLO
     │
     ▼
Application
     │
     ▼
Packet
     │
     ▼
Router
     │
     ├── local → delivery
     │
     └── remote → link 1
                    │
                    ▼
              Reliable Link
                    │
                    ▼
                 Transport
                    │
                    ▼
                   PIO
```

O teste unitário confirma que `SEND DEST=3` originado em P00 seleciona LINK1 e produz um wire frame de 16 bytes.

## Limitação explicitamente mantida

A retransmissão exige cache do frame pendente. A máquina Reliable Link já detecta timeout e retry, mas o cache/reenvio do pacote será fechado na próxima versão.

Isso evita declarar ACK/retry ponta a ponta antes de existir o frame real disponível para retransmissão.
