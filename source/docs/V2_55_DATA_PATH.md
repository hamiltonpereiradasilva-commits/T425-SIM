# V2.55 — caminho completo de transmissão

A partir desta versão, o caminho lógico de uma transmissão é:

```text
Application
    |
    | TX_ID / FINAL_DST
    v
E2E
    |
    v
Router
    |
    | route_select()
    v
NEXT_HOP
    |
    | interface_for_peer()
    v
IF_ID
    |
    v
Reliable Link
    |
    v
PIO State Machine
    |
    v
GPIO / PHY
```

No nó seguinte o processo se repete.

Portanto a mesma mensagem pode percorrer:

```text
P00
  Router -> IF0 -> P01
                   |
                   Router -> IF1 -> P03
                                      |
                                      E2E DELIVERY
```

A aplicação não precisa conhecer GPIO, PIO ou interface física.
