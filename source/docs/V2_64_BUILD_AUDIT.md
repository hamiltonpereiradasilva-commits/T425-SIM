# T425-SIM v2.64 — auditoria de build

A v2.64 não inventa valores do hardware.

Ela introduz uma regra de build explícita:

```text
FLASH_TOTAL_BYTES
APP_FLASH_BYTES
CONFIG_RESERVED = 8192
```

e rejeita a compilação quando:

```text
APP_FLASH_BYTES + 8192 > FLASH_TOTAL_BYTES
```

## Objetivo

A região persistente deve ser conhecida pelo linker e pelo build, e não por
um número mágico espalhado pelo código.

O contrato também exige que exista apenas um mapa de memória autoritativo.

## Resultado

A partir desta versão, antes de habilitar erase/program físico, o projeto
precisa fornecer os valores reais do target escolhido.
