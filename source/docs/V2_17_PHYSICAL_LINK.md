# T425-SIM v2.17 — primeiro enlace físico

A v2.17 deixa de tratar o PIO apenas como infraestrutura e cria um **harness de bring-up** para o primeiro enlace físico.

## Frame de teste

```text
+------+-----+-----+-----+----------+------+
| 0x54 | 0x34| TYPE| SEQ | LEN      | DATA |
+------+-----+-----+-----+----------+------+
                                      │
                                      └── CRC16-CCITT
```

O comando:

```text
PING 1
```

envia um frame de teste no LINK1. O receptor valida CRC e responde com `ACK`.

## Bancada P00 ↔ P01

```text
P00                                  P01
GP6 TX ───────────────────────────► GP5 RX
GP7 RX ◄─────────────────────────── GP4 TX
GND   ───────────────────────────── GND
```

**Não conectar TX diretamente a TX.**

## Sequência de teste

1. Gravar o mesmo UF2.
2. Configurar P00/P01 com `SETID` + `SAVE`.
3. Confirmar `STATUS`.
4. Conectar GND.
5. Conectar TX→RX cruzado.
6. Abrir USB CDC.
7. Em P00 executar `PING 1`.
8. Observar `RX TEST` e `ACK`.

O primeiro objetivo é validar a camada elétrica/PIO e o caminho de bytes. Só depois devemos acoplar o ACK do harness ao mecanismo confiável T425.

## Por que manter o harness separado?

Porque um teste que falha pode então ser classificado como:
- pinagem;
- baud/clock;
- PIO;
- nível elétrico;
- FIFO;
- CRC/frame;
- ou runtime.

Isso evita mascarar falhas de hardware com a complexidade do protocolo completo.
