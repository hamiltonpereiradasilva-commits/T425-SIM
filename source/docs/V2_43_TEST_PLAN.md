# Plano de testes v2.43

## T1 — entrega local

DST = NODE_ID

Esperado:

```text
ACTION = LOCAL_DELIVER
```

## T2 — rota conhecida

No P00:

```text
DST=1 -> HOP=1
```

No P01:

```text
DST=0 -> HOP=0
```

Esperado:

```text
ACTION = FORWARD
```

## T3 — destino sem rota

```text
DST=2
```

nos nós 0/1.

Esperado:

```text
ACTION = DROP
```

## T4 — independência

O módulo t425_router.c não deve depender de:

- PIO;
- GPIO;
- UART;
- CRC;
- timers;
- SDK de comunicação.

Isso permite testar o Router separadamente.

## Próxima validação

Somente depois desses testes será acrescentado o encaminhamento real:

```text
P00 -> P01 -> P02
```

e, posteriormente:

```text
P00
 |  |  P01 P02
  \ /
   P03
```
