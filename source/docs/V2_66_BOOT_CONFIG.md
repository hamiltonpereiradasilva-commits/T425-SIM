# T425-SIM v2.66 — boot e configuração runtime

A v2.66 fecha a regra de inicialização:

```text
BOOT
 ↓
ler SLOT A/B
 ↓
validar
 ↓
selecionar maior generation válida
 ↓
carregar config
 ↓
NODE_ID runtime
 ↓
inicializar interfaces
 ↓
iniciar Router
```

Se nenhum slot for válido:

```text
BOOT → RECOVERY
```

O Router não deve iniciar como se houvesse uma identidade válida.

## Um firmware / N nós

A mesma imagem executável pode ser instalada em qualquer Pico.

A identidade vem da Flash local:

```text
P00 → NODE_ID=0
P01 → NODE_ID=1
P02 → NODE_ID=2
P03 → NODE_ID=3
...
```

Sem recompilar o firmware.
