# T425-SIM v2.28 — correção definitiva da semântica temporal

A falha da v2.27 foi localizada no modelo temporal do Reliable Link.

## Regra implementada

No início de uma transmissão:

```text
tx_time = now
deadline = now + ACK_TIMEOUT
state = WAIT_ACK
```

O `poll(now)` permanece em `WAIT_ACK` enquanto:

```text
now < deadline
```

e muda para `RETRY` quando:

```text
now >= deadline
```

Ao retirar o retry:

```text
tx_time = deadline anterior
deadline = tx_time + ACK_TIMEOUT
state = WAIT_ACK
```

Assim cada retransmissão recebe uma janela de ACK própria.

## Testes

- 109 < 110 → WAIT_ACK
- 110 >= 110 → RETRY
- retry → deadline 120
- 119 < 120 → WAIT_ACK
- 120 >= 120 → RETRY
- ACK correspondente → IDLE
- ACK incorreto → permanece WAIT_ACK
- esgotamento → FAILED

A comparação é feita de forma compatível com overflow de `uint32_t` por comparação signed-difference.
