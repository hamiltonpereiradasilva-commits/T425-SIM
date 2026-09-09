# Próximo teste físico

Depois de compilar o firmware RP2040 com o Pico SDK:

1. mesmo UF2 nos quatro Pico;
2. configurar IDs 0,1,2,3;
3. verificar `STATUS`;
4. conectar GND comum;
5. conectar os enlaces cruzados;
6. iniciar P00, P01, P02, P03;
7. enviar uma mensagem DEST=3;
8. observar os contadores de RX/TX, encaminhamento e entrega.

O próximo incremento deverá expor um comando USB de aplicação, por exemplo:

```text
SEND 3 HELLO
```

O MacBook poderá então injetar uma mensagem na rede e receber o resultado sem conhecer a implementação interna do Router.
