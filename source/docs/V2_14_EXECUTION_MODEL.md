# Modelo de execução

Cada ciclo do scheduler deve futuramente seguir:

```text
1. RX service
2. parser service
3. frame dispatch
4. ACK/retry service
5. routing/service
6. TX service
7. diagnostics
8. timer accounting
```

A ordem acima é o modelo de referência; a implementação atual apenas valida a infraestrutura de ciclo/tick.
