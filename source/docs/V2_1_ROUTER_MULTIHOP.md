# T425-SIM v2.1 — Router multi-hop

A v2.1 adiciona encaminhamento explícito de pacotes entre nós intermediários.

```text
P00                 P01                 P02                 P03
 │                   │                   │                   │
 │ dst=P03           │                   │                   │
 ├──────────────────►│                   │                   │
 │                   │ dst=P03           │                   │
 │                   ├──────────────────►│                   │
 │                   │                   │                   │
```

O endereço lógico original (`SRC`) e a sequência (`SEQ`) são preservados. O campo `DST` passa a representar o próximo salto no enlace físico.

Cada roteador mantém uma tabela:
`destination -> next_hop -> metric`.

## Limite deliberado

A v2.1 usa rotas estáticas. Ainda não há descoberta automática de vizinhos, algoritmo de roteamento dinâmico ou detecção de loops. Esses mecanismos só devem entrar depois que o transporte físico estiver validado.
