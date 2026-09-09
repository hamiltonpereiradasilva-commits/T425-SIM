# T425-SIM RP2040 — v1.1

## Entregue

- parser incremental de stream;
- nó de link com dois links lógicos;
- contadores RX;
- plano de pinagem inicial;
- procedimento de bring-up;
- teste de pacote em stream;
- teste de recepção por nó.

## Não declarado como validado

O transporte elétrico entre dois Pico ainda depende de teste em bancada. A pinagem documentada é uma proposta e deve ser conferida contra a revisão exata da placa.

## Próxima etapa

Implementar o driver PIO real no firmware Pico, com SM TX/RX por link, DMA/FIFO quando necessário, e telemetria USB para o MacBook.
