# Bring-up v2.3

1. Gravar P00.
2. Abrir console USB no MacBook.
3. Confirmar `T425 P0 READY`.
4. Repetir para P01, P02 e P03.
5. Somente depois habilitar PIO TX/RX.
6. Testar P00↔P01 em loopback/ponto a ponto.
7. Integrar CRC.
8. Integrar reliable link.
9. Integrar RDV e router.

Não declarar comunicação distribuída funcional antes dos testes 6–8.
