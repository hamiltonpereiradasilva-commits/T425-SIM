# T425-SIM v2.50 — tráfego bidirecional

A v2.50 prepara a validação da rede em ambos os sentidos:

```text
P00 <-> P01 <-> P02 <-> P03
```

## Casos

```text
P00 -> P03   primeiro salto P01
P03 -> P00   primeiro salto P02
P01 -> P03   primeiro salto P02
P02 -> P00   primeiro salto P01
P00 -> P02   primeiro salto P01
P03 -> P01   primeiro salto P02
P01 -> P00   primeiro salto P00
P02 -> P03   primeiro salto P03
```

O mesmo firmware continua sendo usado em todos os nós.

## O que muda conceitualmente

Até a v2.49, a discussão principal era uma mensagem seguindo um caminho.

Agora testamos a propriedade de que cada nó consegue atuar simultaneamente como:

- origem;
- destino;
- encaminhador.

## Importante

O arquivo `t425_bidirectional_test.c` é um teste determinístico do plano de roteamento.
Ele não deve ser interpretado como prova de tráfego físico. A prova física requer quatro
Pico, cabeamento real e observação dos consoles USB.

## Próximo teste físico

Depois da matriz de rotas:

```text
P00 -> P03
P03 -> P00
```

deve-se executar tráfego simultâneo ou intercalado, verificando:

```text
ACK hop-by-hop
SEQ por enlace
TX_ID fim-a-fim
DELIVERED no destino
DUPLICATE sem dupla entrega
```
