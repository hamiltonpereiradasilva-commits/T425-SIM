# V2.65 — próximo passo

Com a identidade de nó retirada do compile time, o próximo passo é ligar a
sequência completa de boot:

```text
USB/Flash
   ↓
load config
   ↓
validate
   ↓
runtime node_id
   ↓
initialize interfaces
   ↓
start router
```

Depois disso, a gravação física de Flash pode ser integrada sem reintroduzir
identidade de nó no build.
