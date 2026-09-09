# Arquitetura v1.6

```text
OCCAM source
     │
     ▼
 AST / bytecode
     │
     ▼
 OCCAM runtime
     │
     ├── local channel
     │
     └── remote channel
             │
             ▼
       reliable link
             │
             ▼
            PIO
             │
             ▼
           RP2040
```

A separação entre canal lógico e transporte físico é deliberada: permite testar a semântica OCCAM sem depender inicialmente da velocidade final do PIO.
