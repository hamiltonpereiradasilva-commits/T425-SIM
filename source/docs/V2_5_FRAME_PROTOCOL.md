# T425-SIM v2.5 — Frame protocol

O frame completo agora é:

```text
SOF | SRC | DST | TYPE | SEQ | LEN | PAYLOAD | CRC16
 A5    1B    1B    1B    1B    1B     0..240     2B
```

Tamanho total: `8 + LEN` bytes.

O CRC16 cobre `SOF` até o último byte do payload.

## Regras

- `SOF=0xA5`;
- payload máximo: 240 bytes;
- `SEQ` é preservado durante encaminhamento;
- `SRC` representa a origem lógica;
- `DST` é o próximo destino do enlace naquela camada;
- CRC inválido rejeita o frame.

A camada de reliable link deve ficar acima deste framing e não deve modificar o payload lógico.
