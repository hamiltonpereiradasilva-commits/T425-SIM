# Testes v2.5

1. encode/decode de frame;
2. payload vazio;
3. payload máximo;
4. corrupção de cada campo;
5. corrupção do payload;
6. corrupção do CRC;
7. sequência 0..255;
8. encaminhamento preservando SRC/SEQ/PAYLOAD;
9. integração posterior com PIO físico.

O teste de host desta entrega cobre encode/decode e rejeição de payload corrompido.
