# V2.72 — execução durante erase/program

Erase/program da Flash interna não pode ser tratado como uma operação RAM
comum, porque o RP2040 executa normalmente a partir do espaço XIP.

O driver utiliza a seção crítica do Pico SDK ao redor da sequência física.

O projeto deverá ainda validar no hardware a política de interrupções e
concorrência específica do firmware final, especialmente se houver IRQs/PIO
dependentes de código residente em Flash.
