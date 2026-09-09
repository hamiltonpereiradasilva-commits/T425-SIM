# Defeito encontrado na v2.78

Em `t425_config_staging.c`, a função `t425_config_staging_valid()` continha:

```c
s->record.node_id
```

mas `node_id` é membro de `t425_node_config_t`, contido em:

```c
s->record.config.node_id
```

A v2.79 substitui a expressão por:

```c
t425_config_validate(&s->record)
```

Isso é preferível porque a validade do staging passa a depender do mesmo
contrato canônico usado no armazenamento persistente.
