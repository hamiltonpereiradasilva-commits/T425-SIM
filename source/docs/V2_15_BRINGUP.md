# v2.15 — Bring-up físico

1. Grave o UF2 em apenas um Pico.
2. Abra a porta USB CDC.
3. Verifique:
   `T425-SIM v2.15`
   `HW=OK RUNTIME=OK START=OK`
4. Sem ligar P00/P01 ainda, meça GP4/GP6.
5. Ligue GND comum.
6. Para o primeiro enlace P00↔P01:
   - P00 GP6 TX -> P01 GP5 RX
   - P01 GP4 TX -> P00 GP7 RX
   - GND -> GND
7. Comece em 115200 baud.
8. Só depois valide ACK/CRC/retry.
