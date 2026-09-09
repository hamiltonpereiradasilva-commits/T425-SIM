# T425-SIM v2.61 — mapa e contrato de Flash

A v2.61 estabelece o contrato físico sem ainda escolher um endereço absoluto
dentro da imagem final.

Parâmetros:

```text
slot size   = 4096 bytes
erase unit  = 4096 bytes
program unit= 256 bytes
```

Dois slots consecutivos são exigidos:

```text
SLOT A
offset = BASE

SLOT B
offset = BASE + 4096
```

Ambos devem respeitar alinhamento de 4096 bytes.

## Por que BASE continua abstrato?

O endereço absoluto depende do tamanho real da imagem e do linker script do
projeto. Escolher um endereço arbitrário pode fazer o firmware sobrescrever a
si próprio.

Portanto a próxima integração deve reservar explicitamente:

```text
APPLICATION IMAGE
        |
        v
CONFIG SLOT A
        |
        v
CONFIG SLOT B
```

no mapa final de Flash.
