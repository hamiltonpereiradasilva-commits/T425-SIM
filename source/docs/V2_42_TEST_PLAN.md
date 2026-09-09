# Plano de testes v2.42

### T1 — normal
TX/RX/ACK crescentes; RETX=0; TIMEOUT=0; DUP=0; OOO=0.

### T2 — ACK perdido
Forçar perda de um ACK.
Esperado: RETX>0; receptor contabiliza DUP; entrega única.

### T3 — frame corrompido
Usar a injeção da v2.40.
Esperado: CRC>0; RETX>0.

### T4 — enlace interrompido
Desconectar retorno por tempo superior a 5*50 ms.
Esperado: TIMEOUT>0.

### T5 — recuperação
Restabelecer enlace.
Esperado: novos frames/ACKs e sequência continuar.

### T6 — OOO
Injetar SEQ não esperado.
Esperado: OOO>0 e nenhuma entrega duplicada.

Fiação:
P00 GP2 TX -> P01 GP3 RX
P00 GP3 RX <- P01 GP2 TX
GND -> GND
