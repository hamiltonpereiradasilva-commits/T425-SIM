# V2.57 — critérios de aceitação

1. O mesmo firmware deve aceitar configurações com NODE_ID diferentes.
2. O registro deve possuir MAGIC, VERSION, LENGTH e CRC32.
3. Um registro intacto deve validar.
4. Alterar NODE_ID sem recalcular CRC deve invalidar o registro.
5. Recriar o CRC deve restaurar a validade.
6. A futura escrita USB deve ser atômica em relação à configuração ativa.
7. Nenhuma alteração de NODE_ID deve exigir recompilação do protocolo.
