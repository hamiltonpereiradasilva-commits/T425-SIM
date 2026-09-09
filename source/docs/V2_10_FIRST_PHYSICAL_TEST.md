# Primeiro teste físico P00 ↔ P01

## Fase A — sem retry

1. P00 envia DATA.
2. P01 recebe e valida CRC.
3. P01 gera ACK com o mesmo SEQ.
4. P00 recebe ACK.
5. estado retorna a IDLE.

## Fase B — ACK removido

Desconectar temporariamente o RX de P00 ou bloquear ACK no P01.
O P00 deve:
- entrar em timeout;
- retransmitir;
- repetir até MAX_RETRIES;
- entrar em FAILED.

## Fase C — recuperação

Restabelecer ACK antes do limite.
O P00 deve voltar a IDLE ao receber o ACK correto.

Não validar taxas altas antes de confirmar a integridade elétrica e temporização no osciloscópio/analisador lógico.
