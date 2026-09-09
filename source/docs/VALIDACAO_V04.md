# Validação v0.4

Teste integrado:
1. inicializar P00;
2. criar P01 com prioridade 1;
3. verificar seleção de P01;
4. executar IN(c) sem escritor e verificar WAIT_CHAN;
5. executar OUT(c,456) pelo processo parceiro;
6. verificar rendezvous e desbloqueio;
7. repetir no sentido inverso com OUT antes de IN;
8. executar END/STOP/RUN sobre processos.

Resultado esperado do teste host-side: `runtime process/scheduler/channel smoke: PASS`.
