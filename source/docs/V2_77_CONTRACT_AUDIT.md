# T425-SIM v2.77 — auditoria de contratos

A revisão da v2.76 encontrou quatro problemas que impediriam uma integração
honesta:

1. **CMake/linker duplicado:** havia duas estratégias concorrentes para
   `pico_flash_region.ld`. A v2.77 deixa uma única estratégia.
2. **Símbolos do linker:** um arquivo `config_symbols.ld` era adicionado como
   override sem ser o script que o linker inclui. Os símbolos agora são
   anexados ao `pico_flash_region.ld` efetivamente usado.
3. **Primeiro COMMIT:** a transação rejeitava `A=invalid, B=invalid`, embora
   esse seja o estado esperado de uma unidade virgem. A primeira gravação
   passa a criar `SLOT A, generation=1`.
4. **API de configuração:** `main.c` usava `t425_config_store_save/load/clear`,
   mas essas funções não estavam implementadas no módulo correspondente.
   A v2.77 fecha esse contrato.

Também foi mantido o endereço relativo exigido pelas APIs `flash_range_*`:
o driver converte endereço XIP para offset removendo `XIP_BASE`.
