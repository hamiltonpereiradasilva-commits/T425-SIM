# T425-SIM RP2040 — v1.0

## Objetivo

Transformar os quatro Pico P00–P03 em uma rede experimental de nós T425, mantendo a VM/semântica OCCAM da v0.9 e acrescentando uma fronteira explícita de comunicação física.

## Arquitetura

```text
MacBook Pro
    │ USB/Wi-Fi (controle)
    ▼
Host / OCCAM toolchain
    │
    ├───────────────┐
    ▼               ▼
 P00 ───────────── P01
  │                 │
  │                 │
 P03 ───────────── P02
```

A primeira topologia de laboratório é um anel lógico. A camada de pacote não depende da topologia e poderá posteriormente suportar malha.

## Camadas

1. OCCAM subset
2. AST + análise semântica
3. Process images
4. Scheduler
5. VM
6. Logical channels
7. Link packet
8. PIO byte transport
9. GPIO/nível físico

## Pacote de link

```text
+-----+-----+-----+------+------+------+---------+-----+
| SOF | SRC | DST | TYPE | SEQ  | LEN  | PAYLOAD | CRC |
+-----+-----+-----+------+------+------+---------+-----+
  1     1     1     1     1      1      0..240     1
```

CRC v1.0 é soma modular de todos os bytes do cabeçalho e payload. É deliberadamente simples para o primeiro bring-up; uma revisão posterior pode usar CRC-8/CRC-16.

## Separação Host/Firmware

A codificação/decodificação de pacotes é compilável no host e testável sem hardware. O driver PIO fica isolado atrás de `t425_pio_links.*`.

Isso permite testar primeiro:

`OCCAM → VM → channel → packet → packet parser`

e somente depois:

`packet → PIO → GPIO → PIO → packet`.

## Limitações v1.0

A implementação entregue nesta etapa **não declara o link PIO como eletricamente validado**. O `pio/link_uart.pio` existente é a base de byte transport; a validação elétrica depende da placa, pinagem, nível lógico, cabeamento e osciloscópio/analisador lógico.

## Próximo marco

v1.1 deverá implementar:
- TX/RX PIO real;
- FIFO por link;
- framing em tempo real;
- ACK/retransmissão;
- timeout;
- identificação automática dos quatro nós;
- teste P00→P01→P02→P03;
- interface MacBook para telemetria.
