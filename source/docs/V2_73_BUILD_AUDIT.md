# V2.73 — auditoria antes do primeiro UF2

Antes de considerar o firmware físico pronto:

1. compilar com o Pico SDK;
2. confirmar que o linker aceita o fragmento;
3. inspecionar `.map`;
4. verificar `__t425_config_start__` e `__t425_config_end__`;
5. confirmar que nenhum código/dado do firmware invade os 8192 bytes;
6. gerar `.uf2`;
7. programar primeiro um único Pico;
8. testar read/write/reboot;
9. somente depois duplicar o mesmo UF2 para P00/P01.

A propriedade multi-nó não deve ser validada antes da persistência individual.
