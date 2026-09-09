# Teste crítico v2.49

Cenário:

```text
P00 -> P01 -> P02 -> P03
```

1. P00 envia TX_ID=42.
2. P03 entrega TX_ID=42.
3. ACK E2E é perdido.
4. P00 permanece PENDING.
5. P00 retransmite.
6. P03 recebe TX_ID=42 novamente.
7. P03 NÃO entrega novamente à aplicação.
8. P03 pode reenviar ACK E2E.
9. P00 passa para DELIVERED.

Este teste é obrigatório antes da camada OCCAM.
