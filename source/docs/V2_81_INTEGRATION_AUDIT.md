# T425-SIM v2.81 — integração de configuração/runtime

A v2.80 revelou uma segunda família de inconsistências entre módulos antigos
e a configuração atual.

## Correções

### Node configuration
O CMake estava compilando `src/t425_node_config.c`, mas o header selecionado
pelo target descreve `t425_node_config_t` como:

```text
node_id + interfaces
```

O source antigo esperava:

```text
node_count_hint + link_baud + links[]
```

A v2.81 usa `firmware_pico/t425_node_config.c`, que implementa a API atual.

### Runtime
`src/t425_runtime.c` ainda acessava `config.links[]` e `T425_MAX_LINKS`.
Foi adaptado para `config.interfaces.items[]` e `T425_IF_DOWN`.

`T425_MAX_LINKS` foi mantido como alias de compatibilidade para
`T425_MAX_INTERFACES`, evitando quebrar a estrutura do runtime.

### Main
`main.c` ainda acessava `cfg.link_baud`, campo inexistente na configuração
atual. O baud físico agora vem da definição de build:

```text
T425_LINK_BAUD
```

## Gate

Os sources CMake-listados que não dependem diretamente dos headers do Pico
foram novamente submetidos a GCC host com:

```text
-Wall -Wextra -Werror -fsyntax-only
```

A auditoria não deve ser confundida com o build ARM real.
