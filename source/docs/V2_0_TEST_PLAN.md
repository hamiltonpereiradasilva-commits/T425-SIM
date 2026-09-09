# Plano de teste v2.0

Teste mínimo validado no host:
P00 -> canal 1 -> P01.

Extensão planejada:
P00 -> P01 -> P02 -> P03.

Critérios:
1. cada salto deve preservar SRC/DST/CHAN/SEQ;
2. ACK de transporte não pode ser confundido com RDV_ACK;
3. processos devem bloquear até a conclusão da operação;
4. uma falha em um salto não pode duplicar a mensagem lógica.
