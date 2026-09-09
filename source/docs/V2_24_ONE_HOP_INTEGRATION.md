# T425-SIM v2.24 — One-hop integration

A v2.24 integra Transport, Reliable Link e uma interface PHY abstrata em um teste de um salto.

## Fluxo

```text
P00 Application
      │
      ▼
 Reliable Link
      │
      ▼
 Transport + CRC
      │
      ▼
 PHY TX
      │
      ═══════════════
      │
      ▼
 PHY RX
      │
      ▼
 Transport + CRC
      │
      ▼
 Reliable Link
      │
      └── ACK(seq)
             │
             ▼
           P00
```

O teste de loopback confirma:
- frame de dados transmitido;
- frame recebido e validado;
- ACK retornado com a mesma SEQ;
- transação encerrada;
- frame inválido rejeitado e contado.

A PHY continua abstrata por callback. A substituição desse callback pela PIO do RP2040 é o próximo passo físico.
