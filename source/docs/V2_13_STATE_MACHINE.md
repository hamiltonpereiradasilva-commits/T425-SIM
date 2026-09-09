# Estados do runtime

```text
BOOT ──init──► READY ──start──► RUNNING
  │                              │
  └────────────error────────────► ERROR
```

A v2.13 concentra a infraestrutura de estado, mas o comando explícito que leva `READY` a `RUNNING` será conectado ao supervisor/CLI na próxima etapa.
