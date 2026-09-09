# v2.19 — testes

### Teste 1 — local
DEST == NODE_ID → entrega local.

### Teste 2 — vizinho
P00 → P01 utiliza LINK1.

### Teste 3 — múltiplos saltos
P00 → P03:
P00/LINK1 → P01/LINK1 → P02/LINK1 → P03.

### Teste 4 — TTL
TTL=0 em destino remoto → descarte.

### Teste 5 — destino inexistente
Sem rota → descarte e contador `dropped_no_route`.

O teste unitário do Router deve passar antes do teste físico.
