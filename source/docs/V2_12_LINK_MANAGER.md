# T425-SIM v2.12 — Generic Multi-Link Manager

A v2.12 introduz o `Link Manager`, responsável por tratar os links habilitados pela tabela de configuração.

```text
                Generic Node
                     │
               Link Manager
          ┌──────────┼──────────┐
          ▼          ▼          ▼
       LINK 0     LINK 1     LINK 2 ... LINK 3
          │          │          │
         PIO        PIO        PIO
          │          │          │
        GPIO       GPIO       GPIO
```

O manager não conhece P00, P01, P02 ou P03 como casos especiais.

Cada link é selecionado por `local_link`, e o vizinho vem de `neighbor_id`.

## Observação de integração

O módulo `t425_link_manager.c` já define a interface de inicialização e de byte I/O para os PIOs. A compilação final como firmware RP2040 exige o Pico SDK e a inclusão correta dos headers/arquivos PIO no projeto de firmware.

O teste fornecido nesta etapa valida a configuração multi-link no host. A validação elétrica e temporal dos PIOs continua sendo responsabilidade do bring-up de bancada.
