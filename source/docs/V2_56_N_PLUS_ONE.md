# Teste N+1

O teste conceitual da v2.56 é adicionar P04.

Não se deve criar:

```text
firmware_P04
```

Deve-se criar somente:

```text
profile_P04
```

com:

```text
NODE_ID=4
INTERFACE_TABLE=...
```

e executar o mesmo firmware.

A partir desta etapa, o próximo trabalho deve retirar os perfis de teste do
código e criar um mecanismo real de configuração persistente.
