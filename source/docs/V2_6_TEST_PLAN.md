# Teste v2.6

Verificar:
1. SEQ inicia em zero;
2. cada transmissão recebe novo SEQ;
3. ACK confirma o mesmo SEQ;
4. timeout incrementa contador;
5. retry incrementa contador;
6. duplicata é detectada;
7. CRC inválido gera estatística;
8. retry máximo é 3.

O teste atual é um teste determinístico de host da máquina de estados. O timeout físico e ACK real dependem da integração PIO.
