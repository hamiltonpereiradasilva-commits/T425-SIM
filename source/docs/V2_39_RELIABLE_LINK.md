# T425-SIM v2.39 — Reliable Link inicial

A v2.39 acrescenta a primeira forma real de confiabilidade sobre o frame CRC.

## Frame

SYNC | VERSION | TYPE | SRC | DST | SEQ | LEN | PAYLOAD | CRC16

TYPE:
- 0x01 DATA
- 0x02 ACK

## Política

É deliberadamente **stop-and-wait ARQ**:

1. transmissor envia DATA;
2. inicia timeout de 50 ms;
3. receptor valida CRC e destino;
4. receptor envia ACK com a mesma SEQ;
5. transmissor recebe ACK e avança SEQ;
6. sem ACK, retransmite até 5 vezes;
7. após esgotar tentativas, registra TIMEOUT e passa ao próximo SEQ.

Isto ainda não é uma janela deslizante. A janela será introduzida somente depois que o stop-and-wait estiver comprovado.

## Observação

O contador CRC precisa ser incrementado quando o parser recebe um frame completo inválido; a versão atual mantém o campo reservado para essa instrumentação. Isso será fechado na v2.40 junto com testes de corrupção deliberada.
