# T425-SIM — Firmware dos Raspberry Pi Pico 0–3

## 1. Papel

Cada Pico representa um nó T425 virtual independente:

- CPU/VM T425;
- RAM interna modelada em 4 KiB;
- registradores A, B, C, W, I e PC;
- scheduler inicial;
- canais;
- timers, em etapa posterior;
- quatro links por PIO;
- interface de gerenciamento UART/RS-485;
- debug por comandos.

O manual do IMS T425 especifica 32 bits, 4 KiB de RAM interna, quatro links
INMOS e suporte de concorrência. O mapa documenta a RAM interna a partir de
`0x80000000`, com `MemStart = 0x80000070`.

## 2. Quatro nós

| Firmware | ID | Função |
|---|---:|---|
| t425_pico0 | 0 | nó T425 0 |
| t425_pico1 | 1 | nó T425 1 |
| t425_pico2 | 2 | nó T425 2 |
| t425_pico3 | 3 | nó T425 3 |

O mesmo código-fonte é usado nos quatro; somente `T425_NODE_ID` muda.

## 3. Pinos

### Links experimentais PIO

| Link | RX | TX |
|---|---:|---:|
| L0 | GP0 | GP1 |
| L1 | GP2 | GP3 |
| L2 | GP4 | GP5 |
| L3 | GP6 | GP7 |

O desenho usa oito state machines, distribuídas pelos dois blocos PIO do RP2040.

### Gerenciamento

- UART0 TX: GP16
- UART0 RX: GP17
- Driver-enable RS-485: GP18
- LED: GP25

A implementação atual usa 1 Mbit/s no transporte PIO experimental. A
velocidade elétrica INMOS do T425 (5/10/20 Mbit/s) deve ser tratada como
uma etapa específica de validação; não se deve confundir este transporte
de laboratório com compatibilidade elétrica do link original.

## 4. VM

A VM implementa a organização básica da instrução T425:

- function codes de nibble;
- PFIX/NFIX;
- A/B/C como evaluation stack;
- W/I/PC;
- operações básicas de memória;
- operações aritméticas/lógicas selecionadas.

A referência T425 informa que operandos maiores que 15 usam prefixação
PFIX/NFIX e que as operações são organizadas por function code.

## 5. Protocolo de gerenciamento

Quadro:

`54 34 VERSION NODE COMMAND LENGTH PAYLOAD CRC`

Comandos previstos:

- PING
- RESET
- RUN
- HALT
- STEP
- READ_REGS
- READ_MEM
- WRITE_MEM
- LOAD
- STATUS

Esse protocolo é deliberadamente simples para permitir que o aplicativo
macOS seja desenvolvido em paralelo.

## 6. Próxima etapa

A sequência recomendada é:

1. validar os quatro firmwares individualmente;
2. validar leitura/escrita de memória;
3. validar execução de pequenos programas;
4. fechar o scheduler;
5. implementar timers;
6. implementar canais bloqueantes;
7. substituir o transporte PIO de laboratório pela implementação do link
   compatível com a temporização/protocolo INMOS;
8. integrar ESP32;
9. integrar T425-SIM Studio.
