# Escalabilidade de nós

O objetivo arquitetural é:

```text
              T425 network

P00 ─── P01 ─── P02 ─── P03 ─── ... ─── Pn
```

O firmware não deve conter lógica específica de P00/P01/P02/P03. O nó é identificado por `T425_NODE_ID`.

Isso prepara a transição para a camada Router, onde a identidade lógica e a topologia serão separadas da PHY.
