# V2.68 — invariantes

Durante qualquer commit:

1. existe um slot ativo válido antes da operação;
2. o alvo é sempre o slot oposto ao ativo;
3. o slot ativo nunca é apagado pelo coordenador;
4. a nova configuração precisa ser válida;
5. falha de escrita não altera o slot ativo;
6. somente uma cópia validada pode substituir a geração ativa.

Essas invariantes são a base dos testes de power-loss.
