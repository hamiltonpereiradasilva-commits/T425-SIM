# T425-SIM v2.16 — um UF2, configuração persistente

A v2.16 elimina a necessidade de compilar P00, P01, P02 e P03 separadamente.

```text
              MESMO UF2
                 │
      ┌──────────┼──────────┐
      ▼          ▼          ▼
    P00         P01        P02 ... PN
      │          │          │
   CONFIG      CONFIG     CONFIG
    FLASH       FLASH      FLASH
```

O comando USB define o ID e grava a configuração:

```text
DEFAULT 1
SAVE
```

ou:

```text
SETID 2
SAVE
```

Na próxima inicialização o nó carrega a configuração persistida.

A configuração inclui o `NODE_ID`, links, vizinhos, GPIOs, PIOs e baud rate.

## Segurança da escrita

A Flash externa é apagada por setor e programada por página. O SDK documenta alinhamento de 4096 bytes para erase e 256 bytes para program; também alerta que operações de flash exigem sincronização adequada com execução XIP/segundo core. Por isso a implementação usa `flash_safe_execute`. citeturn0search0turn0search1

A região usada é o último setor da Flash (`PICO_FLASH_SIZE_BYTES - 4096`). Isso deve ser reservado ao firmware e não utilizado por outro armazenamento.

## Comandos

```text
HELP
STATUS
GETID
SETID n
DEFAULT n
SAVE
LOAD
CLEAR
START
```

`SETID` altera a configuração em RAM; `SAVE` torna a alteração persistente.

## Importante

A v2.16 já suporta o conceito de um único UF2, mas a topologia default continua sendo a bancada linear P00..P03. Topologias arbitrárias serão configuradas posteriormente pelo protocolo de gerenciamento.
