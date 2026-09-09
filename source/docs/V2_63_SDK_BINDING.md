# V2.63 — vinculação ao Pico SDK

Para liberar a implementação física é necessário fixar no projeto:

1. versão do Pico SDK;
2. placa/target RP2040;
3. linker script efetivamente usado pelo build;
4. símbolos `__t425_config_start__` e `__t425_config_end__`;
5. tamanho efetivo da imagem;
6. região reservada de 8192 bytes.

Somente então o driver deve chamar as primitivas oficiais de erase/program.

A interface do driver já está isolada, de modo que essa vinculação não altera
o Router, o protocolo USB ou a configuração N-nodes.
