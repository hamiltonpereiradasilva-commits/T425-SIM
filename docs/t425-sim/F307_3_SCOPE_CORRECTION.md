# F307.3 — Correção de escopo Git

## Escopo ativo

- `source/t425_control_center/`: aplicativo operacional T425 Visual Control Center.
- `source/linkusb/`: componente produtivo LINKUSB.

## Fora do escopo

`tools/t425_visual_debugger/` permanece preservado exatamente como encontrado.
Seus fontes não foram copiados, excluídos, sobrescritos, staged ou incluídos
em qualquer commit F307.3.

## Correção documental

Os manifests F307.3 devem tratar o Visual Debugger apenas como componente
externo preservado, não como alvo de importação, commit ou promoção nesta
consolidação. O commit atual contém somente esta correção documental.

## Integridade

- `main` não foi movida.
- Referências F307.2B permanecem preservadas.
- Nenhum reset, clean ou push foi executado.
- O índice de trabalho preexistente foi preservado através de índice isolado.
