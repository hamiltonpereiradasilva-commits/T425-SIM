# T425-SIM v2.25 — One-hop corrected integration

A v2.25 corrige e fecha o teste de um salto iniciado na v2.24.

## Correções

1. O endpoint agora conhece `node_id`.
2. ACK usa destino interno explícito `0xFE`.
3. O ACK carrega a mesma `SEQ` do frame confirmado.
4. O pacote pendente permanece no cache durante `WAIT_ACK`.
5. O timeout coloca o link em `RETRY`.
6. A retransmissão recupera exatamente o pacote pendente.
7. CRC inválido é contado e rejeitado.

## Cenário validado

```text
P00 ── DATA(SEQ=0) ──► P01
P00 ◄── ACK(SEQ=0) ─── P01
```

Depois:

```text
P00 ── DATA(SEQ=1) ──► P01
       ACK perdido
P00       TIMEOUT
P00 ── DATA(SEQ=1) ──► P01
```

A retransmissão não cria uma nova sequência.

## Próximo passo

Substituir o callback PHY de teste pelo driver PIO real do RP2040 e manter exatamente esta interface acima dele.
