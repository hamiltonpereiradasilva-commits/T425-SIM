# T425-SIM v2.83 — correções de integridade antes do build

A auditoria da v2.82 mostrou que "CRC válido" não era suficiente para
considerar uma configuração semanticamente válida.

## 1. Validação semântica

Agora `t425_node_config_validate()` verifica:

- quantidade de interfaces <= 4;
- `if_id` sequencial;
- `peer_id` não reservado (`0xFF`);
- PIO SM dentro de 0..7;
- GPIO dentro de 0..29;
- estado somente DOWN/UP;
- nenhum `peer_id` duplicado.

A construção do registro persistente recusa configuração inválida.

## 2. CRC

O registro é zerado antes de receber campos, eliminando qualquer dependência
de bytes de padding não inicializados.

## 3. Commit

Após programação, o slot alvo é lido novamente do hardware e somente então
é usado para determinar o novo slot ativo. O `after` retornado representa o
estado efetivamente confirmado no hardware.

## 4. Coordenador único

A implementação física duplicada de `commit` foi removida do driver RP2040.
A única máquina transacional é `t425_flash_transaction()`.

## 5. Tamanho

Foi adicionado `_Static_assert` garantindo que toda a estrutura do slot cabe
em 4096 bytes.
