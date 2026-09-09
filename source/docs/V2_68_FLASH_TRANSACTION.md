# T425-SIM v2.68 — coordenador transacional de Flash

A v2.68 fecha a política transacional em uma única função:

```text
t425_flash_transaction()
```

Sequência:

```text
current
  ↓
select active
  ↓
select inactive target
  ↓
prepare candidate
  ↓
HW: erase target
  ↓
HW: program target
  ↓
HW: read-back / validate
  ↓
accept new generation
```

A camada de transação nunca chama diretamente as primitivas do RP2040.

Ela utiliza:

```text
t425_flash_hw.c
        ↓
t425_flash_rp2040_real.c
```

Isso permite testar a política independentemente do hardware.
