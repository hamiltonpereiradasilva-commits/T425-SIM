# V2.65 — um firmware para N nós

Princípio:

```text
             SAME FIRMWARE IMAGE
                    |
        ┌───────────┼───────────┐
        ▼           ▼           ▼
      P00         P01         P02 ... PN
        |           |           |
      CFG=0       CFG=1       CFG=2
```

Somente parâmetros de hardware/configuração variam por unidade.

O protocolo, scheduler, runtime, Router, transportes e PIO continuam sendo
componentes do mesmo firmware.
