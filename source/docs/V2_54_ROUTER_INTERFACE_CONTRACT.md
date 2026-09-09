# Contrato Router ↔ Interface Manager

Fluxo:

```text
Router:
  NEXT_HOP = 2
       |
       v
Interface Manager:
  lookup(peer=2)
       |
       v
  IF1
       |
       +--> PIO SM1
       +--> TX GPIO
       +--> RX GPIO
```

Se a interface estiver DOWN:

```text
lookup(peer=2) -> INVALID
```

O Router não deve transmitir por uma interface DOWN.

A próxima etapa deve integrar diretamente:

```text
route_select()
        +
interface_for_peer()
        +
link_transmit()
```

e realizar a primeira transmissão redundante realmente direcionada por
interface física.
