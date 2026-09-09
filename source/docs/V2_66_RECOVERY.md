# V2.66 — modo Recovery

Recovery é um estado explícito, não um `NODE_ID` padrão.

Quando A e B são inválidos:

```text
NODE_ID = NÃO CONFIGURADO
ROUTER   = NÃO INICIADO
```

O dispositivo deve permanecer acessível pelo canal de manutenção/USB para
receber uma configuração válida.

Depois de um commit válido:

```text
USB CONFIG
    ↓
FLASH
    ↓
REBOOT
    ↓
CONFIGURED
    ↓
NODE ONLINE
```
