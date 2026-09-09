# Plano de teste físico v2.18

## Teste A — sucesso
P00 envia PING(seq=N), P01 valida CRC e responde ACK(seq=N).

Esperado:
```text
P00 TX
P01 RX
P01 ACK
P00 ACKED
```

## Teste B — CRC
Alterar um byte do frame.

Esperado:
```text
RX rejected
NO ACK
TIMEOUT
RETRY
```

## Teste C — perda de ACK
Impedir temporariamente a resposta.

Esperado:
```text
WAIT_ACK
TIMEOUT
RETRY 1
TIMEOUT
RETRY 2
FAILED
```

## Teste D — ACK atrasado/duplicado
ACK de sequência antiga não deve encerrar uma nova transação.
