# T425-SIM v2.54 — gerenciador de interfaces físicas

A v2.54 cria a associação explícita:

```text
NEXT_HOP
   |
   v
PEER_ID
   |
   v
INTERFACE
   |
   +-- PIO state machine
   +-- TX GPIO
   +-- RX GPIO
   +-- interface state
```

Exemplo em P00:

```text
IF0 -> P01 -> PIO SM0 -> TX GPIO2 / RX GPIO3
IF1 -> P02 -> PIO SM1 -> TX GPIO4 / RX GPIO5
USB -> MacBook
```

Os GPIOs são exemplos de teste e não constituem ainda o pinout elétrico final.

## Regra

O Router escolhe:

```text
NEXT_HOP=P02
```

e o gerenciador resolve:

```text
P02 -> IF1
```

Se IF1 estiver DOWN:

```text
P02 -> nenhum IF elegível
```

O Router deverá então escolher outro candidato de rota, se existir.

## Separação

A tabela de interfaces não decide a rota.

Ela responde somente:

> "Qual interface física atualmente UP está associada a este peer?"

Isso mantém Router e hardware desacoplados.
