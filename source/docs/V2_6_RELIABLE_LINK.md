# T425-SIM v2.6 — Reliable Link

A camada reliable agora possui estado explícito para:

- sequência de transmissão (`SEQ`);
- ACK;
- retry;
- timeout;
- duplicatas;
- estatísticas de enlace;
- erros de CRC.

## Modelo

```text
TX frame
   │
   ▼
wait ACK
 ┌─┴──────────────┐
 │                │
ACK              timeout
 │                │
 ▼                ▼
done          retry <= 3
                  │
                  └── excedeu → erro
```

A v2.6 fornece o modelo de estado e contadores. O temporizador baseado no hardware e a fila de frames serão ligados ao PIO na próxima etapa.
