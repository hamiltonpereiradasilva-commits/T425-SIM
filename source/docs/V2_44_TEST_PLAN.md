# Plano v2.44

## T1 — linha de três nós

```text
P00 ---- P01 ---- P02
```

Rota:

```text
0 -> 1 -> 2
```

Esperado:

```text
P00 FORWARD HOP=1
P01 FORWARD HOP=2
P02 LOCAL_DELIVER
HOPS=2
```

## T2 — TTL

Criar uma rota em ciclo e iniciar com TTL=2.

Esperado: o frame não deve permanecer encaminhando indefinidamente.

## T3 — retorno

```text
P02 -> P01 -> P00
```

Esperado: decisões simétricas.

## Próximo passo

Depois da prova de cabeçalho/decisão, integrar o forwarding ao transporte físico,
fazendo P01 receber, validar e retransmitir o payload para P02.
