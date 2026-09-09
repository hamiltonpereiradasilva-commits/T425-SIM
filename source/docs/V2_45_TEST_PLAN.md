# Plano de testes v2.45

## T1 — multi-hop lógico

Esperado:

```text
P0 RESULT=SEND_NEXT
P1 RESULT=SEND_NEXT
P2 RESULT=DELIVER
SENDS=2
```

## T2 — TTL esgotado

Iniciar com TTL=1 e tentar dois saltos.

Esperado: DROP no segundo encaminhamento.

## T3 — rota ausente

Remover a rota de P01 para P02.

Esperado: ROUTE DROP.

## T4 — próximo passo físico

Substituir `test_send()` pelo adaptador:

```text
forward engine
      |
      v
reliable link
      |
      v
PIO PHY
```

Sem alterar a lógica de decisão do Router.
