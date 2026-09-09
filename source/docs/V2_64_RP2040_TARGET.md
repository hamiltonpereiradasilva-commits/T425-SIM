# V2.64 — target RP2040

A camada de persistência não deve assumir uma capacidade de Flash sem que o
target da placa esteja definido.

A identificação necessária é:

```text
MCU      = RP2040
BOARD    = target efetivo do projeto
FLASH    = capacidade física do target
SDK      = versão efetivamente utilizada
LINKER   = script efetivamente utilizado
```

Esses dados devem alimentar o build.

Não é permitido substituir a descoberta do target por um endereço fixo de
Flash no código.
