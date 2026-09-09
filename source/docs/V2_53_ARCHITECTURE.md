# V2.53 — arquitetura consolidada

```text
                 E2E
                  |
               TX_ID
                  |
                ROUTER
                  |
        +---------+---------+
        |                   |
   candidate A         candidate B
        |                   |
     health A            health B
        |                   |
        +---------+---------+
                  |
            Reliable Link
                  |
                PIO PHY
```

A métrica escolhe entre candidatos que estejam UP.

A saúde do enlace pode eliminar temporariamente um candidato sem destruir
sua configuração de rota.

Quando o enlace se recupera, o candidato volta a ser elegível.

Isso prepara o caminho para descoberta dinâmica de vizinhos e métricas mais
ricas, sem misturar descoberta, roteamento e transporte.
