# T425-SIM v1.6 — Canais OCCAM sobre o enlace

A v1.6 cria a primeira ponte entre o modelo de comunicação OCCAM e o protocolo físico.

## Modelo

```text
PROC A                         PROC B
  │                              │
  │ OUT(chan, value)             │
  ├────────── packet ───────────►│
  │                              │ IN(chan, value)
```

O pacote `LOUT` carrega:
- `SRC`;
- `DST`;
- `SEQ`;
- `CHAN_ID`;
- payload.

A camada ainda não é um compilador OCCAM completo. É a ABI mínima para que o runtime possa transportar `OUT` remoto e, posteriormente, implementar `IN`, rendezvous e bloqueio.

## Próxima extensão

1. fila de canais;
2. `IN` remoto;
3. rendezvous OCCAM;
4. scheduler de processos;
5. semântica de bloqueio;
6. parser/AST do subconjunto OCCAM.
