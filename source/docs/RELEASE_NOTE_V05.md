# Release note v0.5 corrigida e validada

Foram corrigidas duas inconsistências de integração detectadas durante a validação:

1. o descritor de processo do runtime passou a usar `t425_runtime_process_t`, evitando colisão com o descritor arquitetural da VM;
2. o teste de runtime foi ajustado para distinguir o processo inicial da VM da tabela de processos do runtime.

Os três testes host-side foram recompilados e executados com `-Wall -Wextra`.
