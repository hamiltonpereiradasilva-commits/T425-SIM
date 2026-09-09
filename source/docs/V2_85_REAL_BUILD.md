# T425-SIM v2.85 — resultado do primeiro gate real

O gate foi efetivamente executado no ambiente desta sessão.

O resultado foi registrado em `V2_85_REAL_BUILD_RESULT.txt`.

A execução não deve ser interpretada como um build ARM bem-sucedido se o
ambiente não possuir `arm-none-eabi-gcc` e um `PICO_SDK_PATH` válido. Nesse
caso o gate termina antes do CMake ARM e registra explicitamente o motivo.

Também foi corrigido o script para passar `-DPICO_SDK_PATH=...` ao CMake,
além da variável de ambiente.

O próximo ambiente que contenha SDK + toolchain poderá executar exatamente o
mesmo gate e produzir ELF/MAP/BIN/UF2.
