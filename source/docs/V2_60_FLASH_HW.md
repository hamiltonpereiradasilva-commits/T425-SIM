# T425-SIM v2.60 — camada física de Flash RP2040

A v2.60 cria a fronteira entre a política transacional e o hardware.

## Política

Continua em:

```text
t425_flash_slots.c
```

## Hardware

Passa a ser isolado em:

```text
t425_flash_hw.c
```

A camada física deverá posteriormente definir:

- região reservada da Flash;
- offsets dos slots;
- alinhamento de erase;
- tamanho das unidades de programação;
- rotina de erase;
- rotina de program;
- leitura de verificação;
- tratamento de falha.

## Segurança

Nesta versão o driver é deliberadamente não destrutivo.

Nenhum endereço de Flash foi inventado e nenhuma rotina de erase é executada.

Isso evita corromper o firmware existente enquanto o mapa de memória do projeto
não estiver formalmente definido.
