# T425-SIM v1.5 — enlace confiável

A camada v1.5 acrescenta confiabilidade sobre o framing:

```text
DATA(seq)
   │
   ├──── ACK(seq) ────► concluído
   │
   └── timeout
          │
          ├─ retry 1
          ├─ retry 2
          ├─ retry 3
          └─ falha
```

Parâmetros iniciais:
- máximo de 3 retransmissões;
- timeout lógico de 100 ticks;
- apenas um DATA pendente por link lógico.

A implementação ainda é deliberadamente simples. Antes de otimizar throughput, devemos validar correção, duplicatas, perda de ACK e recuperação após falha.
