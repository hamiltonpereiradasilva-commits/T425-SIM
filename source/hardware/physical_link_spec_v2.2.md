# Especificação física preliminar v2.2

## Frame

```text
+------+-----+-----+------+-----+----------+-----+
| SOF  | SRC | DST | TYPE | SEQ | LEN      | DATA|
+------+-----+-----+------+-----+----------+-----+
```

O frame lógico deve ser encapsulado pelo driver PIO.

## Handshake

A primeira implementação deve ser half-duplex lógico por par TX/RX, com um transmissor por vez no sentido de cada enlace. Não assumir arbitragem elétrica automática.

## Reset

Após reset:
1. PIO é inicializado;
2. FIFOs são esvaziadas;
3. runtime entra em estado READY;
4. tabela de rotas é carregada;
5. link passa para IDLE.

## Validação de bancada

Antes de quatro nós:
1. P00↔P01;
2. teste loopback;
3. teste packet;
4. teste reliable link;
5. teste RDV;
6. só então adicionar P02 e P03.
