# Topologia física necessária para validar failover

Para haver rota alternativa entre os quatro Pico, uma possibilidade é:

```text
        P01 -------- P02
       /  |          |  \
     P00  |          |  P03
       \  |          |  /
        ----------------
```

A topologia exata será definida na etapa física.

O requisito é simples:

> Deve existir pelo menos um destino para o qual dois primeiros saltos conduzam a
> caminhos fisicamente independentes.

Sem isso, "failover" seria apenas uma decisão lógica sem caminho físico.
