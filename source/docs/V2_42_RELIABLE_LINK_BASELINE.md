# T425-SIM v2.42 — Reliable Link baseline

A v2.42 consolida a máquina stop-and-wait antes da introdução do Router.

## Propriedades

- DATA/ACK;
- CRC16/CCITT;
- sequência de 8 bits;
- detecção de duplicata;
- ACK idempotente;
- timeout;
- retransmissão limitada;
- contador de TIMEOUT;
- parser com contadores de CRC, formato e sincronismo.

## Estado de transmissão

```text
IDLE
 |
 +-- send DATA(seq)
 |
 v
WAIT_ACK
 |       |
ACK    TIMEOUT
 |       |
 v       +--> RETRY (<= MAX_RETRY)
NEXT            |
                +--> TIMEOUT FINAL
```

## Estado de recepção

```text
VALID DATA
    |
    +-- next SEQ -> deliver + ACK
    |
    +-- duplicate -> no deliver + ACK
    |
    +-- out-of-order -> no deliver + ACK/policy
```

## Critério de entrada para Router

Não avançar para Router como "validado" sem executar:
1. operação normal;
2. perda de ACK;
3. CRC inválido;
4. limite de retries;
5. duplicata;
6. recuperação após timeout.

A v2.42 é uma baseline de código; os testes físicos continuam sendo necessários.
