# T425-SIM v2.0 — Runtime distribuído

A v2.0 integra a noção de canal com identificação de nó e rota.

```text
P00                  P01                  P02                  P03
 │                    │                    │                    │
 ├── c01 ───────────►│                    │                    │
 │                    ├── c12 ───────────►│                    │
 │                    │                    │ ├── c23 ─────────►│
 │                    │                    │                    │
```

Cada canal lógico possui:
- `SRC`;
- `DST`;
- `CHAN_ID`.

O transporte remoto usa a camada RDV da v1.9, que por sua vez pode usar o reliable link.

## Limitação desta versão

A topologia de teste é explicitamente roteada por pares. Ainda não existe um roteador multi-hop autônomo; isso será introduzido depois.

## Objetivo seguinte

Adicionar uma tabela de encaminhamento e permitir que P01 encaminhe tráfego destinado a P02/P03, aproximando a rede da estrutura de links do Transputer.
