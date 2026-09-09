# V2.62 — próximo passo do driver físico

Com o linker exportando a região real, o driver poderá calcular:

```text
slot A = CONFIG_START
slot B = CONFIG_START + 4096
```

e implementar:

```text
read_slot()
erase_slot()
program_slot()
verify_slot()
```

A operação de gravação deverá seguir as primitivas oficiais do SDK/ROM do
RP2040 e as restrições de execução durante programação da Flash.

Não devemos inventar uma sequência de baixo nível enquanto o mapa de memória
não estiver fechado.
