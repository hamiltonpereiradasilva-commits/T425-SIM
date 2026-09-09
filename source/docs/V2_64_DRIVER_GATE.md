# V2.64 — gate para liberar o driver físico

O driver físico somente pode ser habilitado quando todos forem verdadeiros:

```text
[ ] target RP2040 confirmado
[ ] capacidade Flash confirmada
[ ] Pico SDK confirmado
[ ] linker oficial identificado
[ ] 8192 bytes reservados
[ ] símbolos start/end exportados
[ ] imagem não sobrepõe configuração
[ ] tamanho/alinhamento validados
```

Enquanto qualquer item estiver pendente:

```text
ERASE/PROGRAM = BLOQUEADO
```

Isso é uma proteção de engenharia, não uma limitação funcional do protocolo.
