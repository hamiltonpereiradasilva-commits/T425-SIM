# Procedimento P00 ↔ P01

1. Compilar firmware para P00 (`T425_NODE_ID=0`).
2. Compilar firmware para P01 (`T425_NODE_ID=1`).
3. Gravar os dois UF2.
4. Conectar GND comum.
5. Conectar P00 GP4→P01 GP5.
6. Conectar P01 GP4→P00 GP5.
7. Abrir duas consoles USB.
8. Verificar boot PIO.
9. Observar PING no analisador lógico.
10. Confirmar PONG.
11. Enviar DATA.
12. Registrar erros.

Não conectar PIOs diretamente sem confirmar que os pinos estão em níveis compatíveis e sem risco de dois drivers ativos no mesmo fio.
