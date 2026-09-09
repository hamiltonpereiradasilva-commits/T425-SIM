# Procedimento de bancada v2.30

1. Gravar o mesmo firmware nos dois Pico, diferenciando apenas `node_id`.
2. Ligar:
   P00 TX → P01 RX
   P00 RX ← P01 TX
   GND ↔ GND
3. Iniciar transmissão de bytes conhecidos: `55 AA 00 FF`.
4. Observar TX com analisador lógico.
5. Confirmar start=0, oito bits LSB-first e stop=1.
6. Confirmar no RX os mesmos bytes.
7. Só então ligar Transport/CRC.
8. Por fim testar ACK e retransmissão.

Critério de aprovação: 100% dos bytes recebidos corretamente em um teste inicial de pelo menos 10.000 bytes, sem perda ou alteração.
