# V2.47 — cabeamento

Enlace 0-1:

```text
P00 GP2 TX -> P01 GP3 RX
P00 GP3 RX <- P01 GP2 TX
```

Enlace 1-2:

```text
P01 GP2 TX -> P02 GP3 RX
P01 GP3 RX <- P02 GP2 TX
```

Enlace 2-3:

```text
P02 GP2 TX -> P03 GP3 RX
P02 GP3 RX <- P03 GP2 TX
```

Todos os GNDs devem ser comuns.

USB:

```text
P00 USB -> MacBook
P01 USB -> MacBook
P02 USB -> MacBook
P03 USB -> MacBook
```

USB é console/telemetria. Não é o caminho dos dados da rede.
