# T425-SIM v2.18 — Reliable Link

A v2.18 acrescenta a máquina de confiabilidade sobre o enlace físico.

```text
              SEND
               │
               ▼
          WAIT_ACK
          │       │
       ACK│       │timeout
          ▼       ▼
         IDLE   RETRY
                   │
             ┌─────┴─────┐
             │            │
          ACK            max retries
             │            │
             ▼            ▼
            IDLE        FAILED
```

Cada transmissão confiável recebe uma sequência de 8 bits.

Parâmetros iniciais:
- timeout: 10 ticks no núcleo de teste;
- máximo: 5 retries;
- sequência monotônica modulo 256.

A camada registra:
- `tx_ok`;
- `rx_ok`;
- retries;
- falhas;
- duplicatas.

## Regra importante

O ACK precisa carregar o mesmo `SEQ` do frame confirmado. ACK de outra sequência não encerra a transação.

## Separação

A v2.18 ainda não transforma o PIO em um protocolo completo de janela deslizante. Há **uma transação confiável pendente por link**. Isso simplifica o primeiro bring-up físico e deixa espaço para posteriormente implementar pipeline/windowing semelhante ao comportamento de enlaces Transputer.
