# Correções da v0.4

A primeira tentativa de validação host-side encontrou uma inconsistência de tipos em `t425_vm.c`: as funções auxiliares de prefixo recebiam `t425_cpu_t*`, mas acessavam o contexto `t425_vm_t`. Isso foi corrigido para que a VM possa ser compilada fora do RP2040 e testada de forma independente.

A versão entregue é considerada válida somente após os testes host-side abaixo passarem.
