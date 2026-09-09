# V2.72 — requisitos do linker

O linker definitivo deve exportar:

```text
__t425_config_start__
__t425_config_end__
```

com:

```text
end - start = 8192 bytes
start % 4096 = 0
end   % 4096 = 0
```

A aplicação não pode ocupar essa região.

Sem esses símbolos, a implementação física não deve ser considerada integrada.
