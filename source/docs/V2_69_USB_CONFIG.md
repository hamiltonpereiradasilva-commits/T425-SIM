# T425-SIM v2.69 — protocolo USB de configuração

A v2.69 cria a fronteira entre o MacBook e o gerenciador de configuração do
Pico.

Comandos definidos:

```text
PING       0x01
GET_CONFIG 0x01
SET_CONFIG 0x02
COMMIT     0x03
REBOOT     0x04
```

Observação: os valores de enumeração devem permanecer como fonte única no
header; a tabela acima representa a intenção do protocolo, e a implementação
deve ser corrigida se houver colisão de códigos.

## Princípio

`SET_CONFIG` não grava Flash.

Somente:

```text
COMMIT
```

pode atravessar a fronteira transacional.

Fluxo:

```text
MacBook
   |
   | SET_CONFIG
   v
RAM / staging
   |
   | validação
   v
COMMIT
   |
   v
transaction
   |
   v
Flash
```

A camada USB não deve conhecer endereços físicos da Flash.
