# Plano de teste v2.1

Cenário:
P00 -> P01 -> P02 -> P03.

Verificações:
- preservação de SRC;
- preservação de SEQ;
- preservação de TYPE;
- preservação de payload;
- alteração de DST para o next-hop;
- tabela de rotas;
- rejeição de rota para o próprio nó;
- seleção da menor métrica para uma rota já cadastrada.

Teste de host atual: P00 -> P01 -> P03.
Teste físico de quatro nós: próxima etapa.
