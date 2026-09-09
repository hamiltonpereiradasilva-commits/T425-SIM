# T425-SIM v0.6 — release note

Durante a validação foi encontrada uma falha de limite na seleção de guardas ALT. A implementação foi corrigida para validar o índice do canal antes do acesso ao vetor.

A versão final também valida a precedência configurada: enquanto o guard de canal está habilitado e pronto, ele é selecionado; quando desabilitado, o guard temporal já disparado é selecionado.

Todos os testes host-side da release final devem retornar código 0.
