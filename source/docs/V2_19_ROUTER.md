# T425-SIM v2.19 — Router

A v2.19 transforma os enlaces independentes em uma rede endereçável.

## Packet

```text
+--------+--------+-----+-----+------+---------+
| DEST   | SOURCE | TTL | SEQ | LEN  | PAYLOAD |
+--------+--------+-----+-----+------+---------+
```

O roteador decide:
- entregar localmente;
- encaminhar para o próximo link;
- descartar por TTL;
- descartar por ausência de rota.

## Topologia da bancada

```text
       LINK0       LINK1       LINK0       LINK1
P00 ─────────── P01 ─────────── P02 ─────────── P03
```

As rotas estáticas iniciais permitem, por exemplo:

```text
P00 → P03
```

ser encaminhado:

```text
P00 → P01 → P02 → P03
```

O TTL é decrementado a cada salto.

## Separação de responsabilidades

```text
Application
    │
    ▼
Packet
    │
    ▼
Router
    │
    ▼
Reliable Link
    │
    ▼
PIO
```

O Router não conhece GPIO nem PIO. O Reliable Link não decide o destino da aplicação.

Essa separação será importante quando introduzirmos OCCAM: processos OCCAM deverão produzir/consumir mensagens, enquanto o runtime de comunicação cuida do transporte.
