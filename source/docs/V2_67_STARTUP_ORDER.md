# T425-SIM v2.67 — ordem de inicialização

A v2.67 fecha a fronteira entre configuração e execução.

## Ordem obrigatória

```text
POWER ON
   ↓
LOAD CONFIG
   ↓
VALIDATE CONFIG
   ↓
RECOVERY se inválida
   ↓
NODE_ID runtime
   ↓
INITIALIZING
   ↓
interfaces
   ↓
ROUTER
   ↓
ONLINE
```

O Router não deve iniciar antes de uma identidade válida e das interfaces
estarem prontas.

## Estados

```text
RECOVERY
INITIALIZING
ONLINE
```
