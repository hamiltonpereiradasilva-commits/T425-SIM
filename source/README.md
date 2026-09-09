# T425-SIM — Nós Raspberry Pi Pico P00–P03 — v0.2

Projeto de firmware para quatro RP2040 que funcionam como nós de uma máquina virtual distribuída inspirada no IMS T425.

## Alvos
- t425_pico0 -> P00
- t425_pico1 -> P01
- t425_pico2 -> P02
- t425_pico3 -> P03

## Conceito
Cada Pico contém uma VM de 32 bits, RAM-modelo de 4 KiB, registradores A/B/C/W/I/PC, scheduler e quatro interfaces PIO reservadas aos links T425. O ESP32 será o supervisor e o MacBook Pro será a estação de engenharia.

## Base documental
A arquitetura e a codificação da ISA seguem o datasheet IMS T425 fornecido no projeto. O manual especifica 4 KiB de RAM interna, MemStart em 0x80000070, quatro links INMOS e a codificação por nibbles PFIX/NFIX.

## Build
Defina PICO_SDK_PATH e execute:

```text
mkdir build && cd build
cmake ..
cmake --build . -j4
```

Serão gerados quatro UF2, um por nó.

## Atenção
v0.2 é uma etapa funcional de desenvolvimento. Não declarar compatibilidade binária ou cycle-accurate com um IMS T425 real até concluir links INMOS, scheduler, canais, bootstrap, timers e a suíte de validação.


## v0.4
VM + runtime concorrente: processos, scheduler e rendezvous de canais estão integrados. O teste host-side `test_runtime` valida a camada sem exigir um RP2040.
