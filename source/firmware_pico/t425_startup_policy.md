# Startup policy v2.65

O `NODE_ID` não deve ser obtido de `T425_NODE_ID` em compile time.

A inicialização correta é:

```text
boot
 ↓
load valid Flash config
 ↓
if valid:
    node_id = config.node_id
else:
    recovery
```

Assim o mesmo UF2 pode ser instalado em qualquer Pico.

A configuração de cada unidade fica armazenada na própria Flash.
