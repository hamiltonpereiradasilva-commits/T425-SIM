# Integração HOSTIO do T425-SIM com o IServer INMOS 1.50v

## Estado

O checkpoint F245 demonstrou a integração funcional em uma variante isolada. Ele não constitui ainda uma release reproduzível sobre a revisão canônica atual. Nenhuma alteração no CORE, CROM, MIR, scheduler, CHANNEL ou PHY foi feita para esta integração.

## Arquitetura

O CORE continua executando instruções pelo caminho microprogramado `OPR → CROM → MIR → microsequenciador → datapath`. A VM mantém uma única sessão com CORE, executor, transporte e host adapter associados ao endpoint `T425SIM0`. O IServer INMOS recebe os argumentos originais por `argc/argv`, constrói as linhas via `MakeCommandLine`/`ParseCommandLine`, executa `Boot → Serve` e é proprietário do despacho HOSTIO e dos handlers de arquivo.

No runner experimental F244/F245, o limite entre guest e IServer transporta bytes em ordem. `OPS_GetRequest`, `Serve`, handlers originais como `SpOpen`/`SpSeek` e `OPS_SendReply` continuam sendo o caminho ativo. O peer de serviços F240 não responde às requisições. O runner experimental ainda usa callbacks host de TX/ACK para a PHY virtual; isso não prova comportamento da PHY física.

## Perfil SP_ID

O IServer 1.50v responde com o perfil original produzido por `SpId`: frame `06 00 00 0F 00 00 00 00`. A execução F240 que serviu de referência anunciava o perfil de compatibilidade `05 00 00 0E 01 01 01`. F245 mostrou, por intervenção isolada, que esse perfil muda o caminho escolhido pelo ILINK e permite chegar ao `SP_OPEN` de `test04.cah`.

Uma integração de release deve expor a escolha na camada host, com dois perfis nomeados: `inmos-1.50` (padrão, sem reescrita) e `compat-v14` (perfil validado em F245). `compat-v14` não é uma correção do CORE nem afirmação de que o perfil original esteja errado para outros alvos.

## Matriz F240/F245

| Caso | ILINK | OCCONF | ICOLLECT | Runtime | Evidência |
|---|---|---|---|---|---|
| T04 | PASS | PASS | PASS | PASS | CAH 1859 B `983c1855…`; CFB 1043 B `2020f4fd…`; BTL 3295 B `0e809905…`; `SP_EXIT=0`, `CPU_ERROR=0` |
| T05 | PASS | PASS | PASS | PASS | CAH 1891 B `3ee79e45…`; CFB 1043 B `44ffd24b…`; BTL 3328 B `fe6d3b5a…`; `SP_EXIT=0`, `CPU_ERROR=0` |
| T06 | PASS | PASS | PASS | PASS | CAH 1919 B `678b8f2a…`; CFB 1043 B `42981284…`; BTL 3356 B `a526d541…`; `SP_EXIT=0`, `CPU_ERROR=0` |

Os artefatos CAH/CFB/BTL F245 coincidem por SHA-256 com F240 para cada caso. Os testes de runtime emitiram `Hello world...` e encerraram com `SP_EXIT` de sucesso.

## Limitações e gates

- F245 executou uma variante isolada, derivada do snapshot F240/F212; a build reproduzível a partir da árvore canônica atual não passou porque a captura/teste F245 referencia campos de CORE e `t425_byte_write.c` ausentes nessa base. Não importar esses deltas como parte da integração HOSTIO.
- A escolha `compat-v14` foi validada nos casos T04–T06, não como perfil universal de todos os programas/alvos.
- O resultado usa PHY virtual e callbacks de host; não valida RP2040 nem ACK físico.
- Não foi feito commit, push ou deploy. A integração não está liberada como release até a reconciliação de base e uma build limpa.
