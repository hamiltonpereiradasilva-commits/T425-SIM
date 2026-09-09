# V2.60 — mapa de Flash

O endereço físico ainda está deliberadamente em aberto.

Antes da implementação destrutiva será necessário definir:

```text
FIRMWARE
CONFIG SLOT A
CONFIG SLOT B
RESERVA
```

e garantir que a área de configuração não sobreponha:

- imagem do firmware;
- dados persistentes existentes;
- espaço reservado pelo linker;
- regiões utilizadas pelo bootloader, quando aplicável.

Somente depois dessa definição o driver deverá receber os offsets reais.
