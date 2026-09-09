# V2.60 — sequência física de commit

A sequência final pretendida é:

```text
1. construir novo registro
2. calcular CRC
3. selecionar slot inativo
4. apagar slot inativo
5. programar registro
6. ler registro de volta
7. validar MAGIC/VERSION/LENGTH/CRC
8. somente então considerar novo slot ativo
```

Nunca:

```text
apagar slot ativo
      ↓
tentar gravar novo
```

O slot antigo deve permanecer intacto até a nova cópia estar validada.
