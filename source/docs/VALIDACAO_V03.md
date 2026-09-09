# Validação v0.3

### ISA
PFIX 3 + LDC 5 -> 0x35; NFIX 1 + LDC 1 -> 0xFFFFFFE1; LDC 10 + LDC 20 + ADD -> 30.

### Canais
- OUT sem leitor: processo fica WAIT_OUT;
- IN sem escritor: processo fica WAIT_IN;
- chegada do parceiro completa o rendezvous;
- valor transmitido é preservado.

### Scheduler
Seleciona processo READY de maior prioridade e salva PC/W do processo corrente.

### Links
Cada L0..L3 possui contadores independentes. O transporte físico INMOS é o próximo marco.
