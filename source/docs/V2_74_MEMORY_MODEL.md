# V2.74 — modelo definitivo de memória

Para Pico padrão com 2 MiB:

```text
0x10000000
    |
    | firmware linkado
    |
    | até 0x101FE000
    |
0x101FE000  __t425_config_start__
    | 4096 B
    | SLOT A
0x101FF000
    | 4096 B
    | SLOT B
0x10200000  __t425_config_end__
```

O firmware não é linkado sobre os últimos 8192 bytes.

A Flash física continua sendo 2 MiB; apenas o espaço disponível para o
firmware é reduzido em 8192 bytes.
