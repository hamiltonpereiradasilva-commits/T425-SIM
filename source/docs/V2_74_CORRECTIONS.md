# T425-SIM v2.74 — correções antes do build

A revisão da v2.73 encontrou dois pontos que precisavam ser corrigidos antes
de qualquer alegação de UF2 funcional.

## 1. Linker

Não é adequado passar um fragmento de região com `-T` como se fosse o script
principal do executável.

A v2.74 usa o mecanismo de override do Pico SDK para `pico_flash_region.ld`,
mantendo o memmap principal do SDK.

O linker passa a enxergar:

```text
FLASH = 2 MiB - 8 KiB
CONFIG = últimos 8 KiB
```

enquanto o hardware continua sendo reconhecido como Flash física de 2 MiB.

## 2. Operações XIP

Erase/program passam por:

```text
flash_safe_execute()
```

e não por uma seção crítica local inventada pelo projeto.

Isso segue o modelo usado pelo Pico SDK para coordenar o uso da Flash quando o
firmware normalmente executa em XIP.

## 3. Fontes

A lista de fontes do alvo é auditada para que os módulos introduzidos nas
versões 2.70–2.73 realmente sejam compilados pelo executável.
