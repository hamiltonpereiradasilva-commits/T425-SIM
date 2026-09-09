# Bring-up v2.16

1. Grave o mesmo UF2 em todos os Pico.
2. Conecte cada Pico ao MacBook individualmente.
3. Pela USB CDC execute:
   - P00: `SETID 0`, `SAVE`
   - P01: `SETID 1`, `SAVE`
   - P02: `SETID 2`, `SAVE`
   - P03: `SETID 3`, `SAVE`
4. Reinicie cada Pico.
5. Execute `STATUS` e confirme o ID persistido.
6. Só então faça as conexões P00↔P01↔P02↔P03.
