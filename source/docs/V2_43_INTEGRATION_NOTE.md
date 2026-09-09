# Integração v2.43

Os arquivos do Router foram adicionados de forma independente para evitar alterar
silenciosamente um CMake existente sem verificar sua estrutura.

Para o próximo passo, o build deverá incluir:

```text
t425_router.c
t425_router.h
```

e um executável de teste contendo:

```text
t425_router_test.c
```

A integração com o executável do Reliable Link será feita na próxima etapa,
quando o caminho P00->P01->P02 for implementado.
