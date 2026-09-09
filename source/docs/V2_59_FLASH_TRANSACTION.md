# T425-SIM v2.59 — Flash transacional em dois slots

A v2.59 implementa a política de dois slots em software:

```text
SLOT A
  config
  generation
  CRC

SLOT B
  config
  generation
  CRC
```

## Seleção no boot

```text
A válida + B válida -> maior generation
A válida + B inválida -> A
A inválida + B válida -> B
A inválida + B inválida -> RECOVERY
```

## Commit

Se A estiver ativa:

```text
gravar B
generation(B) = generation(A)+1
```

Se B estiver ativa:

```text
gravar A
generation(A) = generation(B)+1
```

O slot ativo anterior permanece intacto até que o novo registro seja validado.

## Importante

Esta versão implementa a política e o preparo da transação em C. A rotina de
baixo nível que executa apagamento/escrita física da Flash do RP2040 deverá ser
integrada ao driver de armazenamento antes do ensaio de perda de energia.
