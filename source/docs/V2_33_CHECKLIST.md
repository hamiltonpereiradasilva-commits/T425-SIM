# Checklist de aprovação v2.33

1. Build com Pico SDK e pioasm.
2. Gerar `.pio.h` automaticamente.
3. Gravar P00/P01.
4. Medir GP2 de P00.
5. Confirmar baud real.
6. Confirmar 8N1.
7. Confirmar padrão 55 AA 00 FF 12 34 C3 3C.
8. Confirmar RX.
9. Rodar 10.000 bytes.
10. Registrar erros.
11. Somente depois integrar Transport/CRC.
