# T425-SIM Control Center — F297

Aplicativo desktop nativo para macOS, implementado em Python 3 + Tkinter/ttk. Não usa navegador, servidor HTTP, Flask, Electron ou WebView.

## Segurança da F297

A aplicação abre em DIAGNÓSTICO e não conecta automaticamente. Ao conectar LINKUSB, consulta EP0 B4 somente; não envia Bulk OUT e não lê Bulk IN. A consulta T425 envia somente F271? pela CDC. Load + Serve só pode ser armado com BTL selecionado, F271? recente confirmando RUNNING/Core1 RUNNING/WAIT_CONTROL/sem falha e B4 recente indicando Link0 iniciado, filas vazias, TX pronto e sem ACK/evento pendente. A tela pede confirmação final e o worker revalida o estado e o SHA-256 do BTL antes de iniciar.

O início fecha primeiro a sessão USB diagnóstica PyUSB e seu lock exclusivo; então o IServer original abre uma sessão libusb própria. O resultado do subprocesso é rotulado como `PROCESS_STARTED_NOT_BOOT_CONFIRMED`: iniciar o processo não confirma Bulk OUT, ACK físico, boot concluído ou execução do guest. LinkIO só devolve progresso de WriteLink após incremento ACK_RX observado no B4. Nenhum retry cego é feito.

O IServer executa `Boot()` e depois o `Serve()` original na mesma execução (`-sc <BTL> -ss -sl linkusb0`). Não existe Run separado demonstrado; Run, Stop e Reset continuam desabilitados. ResetLink/AnalyseLink e TestError continuam indisponíveis. A F294 deixou o T425 em LOAD; enquanto F271? não mostrar WAIT_CONTROL, o gate impede o envio. Nenhum reset é feito pelo aplicativo.

O IServer é o IServer INMOS original 1.50v, mantido em iserver/original/. O backend C LinkIO e um roteamento local pequeno em LinkOps foram adicionados em cópias de build isoladas. Boot, Serve, SpCommand, OPS_GetRequest, OPS_SendReply e protocolo SP permanecem do IServer original.

O backend C de WriteLink entrega progresso ao IServer somente depois de observar incremento de ACK_RX via B4. Bulk OUT aceito sozinho não conta como byte escrito. B4 não fornece erro remoto verdadeiro; TestError retorna indisponível (-1). ResetLink e AnalyseLink também retornam indisponível e não são chamados pelo comando preparado. Logo, não usar modo IServer de detecção de erro até haver telemetria válida para esse contrato.

## Requisitos macOS x86_64

- Command Line Tools do Xcode (clang).
- Python do sistema com Tkinter. Neste Mac, /usr/bin/python3 é Python 3.9/Tk 8.5. O Python 3.14 padrão do shell não inclui _tkinter.
- Homebrew libusb e pkg-config: brew install libusb pkg-config
- Python packages PyUSB e PySerial são instalados apenas no venv local.

## Instalação e execução

Na pasta ~/T425_OUTPUT/F297/control_center:

    ./install_macos.sh
    ./run_macos.sh

O launcher executa .venv/bin/python main.py. Não altera o Python global.

Build do IServer original com backend LinkUSB, sem acessar hardware:

    ./iserver/build_macos.sh

O executável resultante é build/bin/iserver-linkusb (Mach-O x86_64). Testes:

    ./tests/run_tests.sh

## Operação da interface

1. Inicie o app. Estado inicial é desconectado.
2. Conectar LINKUSB obtém uma sessão exclusiva, verifica VID:PID 1209:4250, strings de produto/serial quando disponíveis, configuração 1, interface 0 e endpoints Bulk 0x02/0x86. Só então faz polling B4.
3. O polling B4 não consome dados Bulk. A tela mostra bytes USB OUT/IN, DATA/ACK submetidos/decodificados, filas e erros reportados pelo firmware; esses contadores não são captura elétrica.
4. Consultar T425 F271? abre CDC do dispositivo identificado pelo serial 50444672C0A3541C e envia o comando de diagnóstico preexistente. Não executa PEEK/POKE.
5. Selecionar BTL usa o diálogo nativo do Finder e calcula tamanho, SHA-256 e primeiro byte. O Hello World F72 está incluído como examples-hello-t425-test.btl.
6. Consultar ambos os dispositivos e atualizar os snapshots. A tela mostra o primeiro motivo que bloqueia a pré-checagem.
7. Só quando o gate passar, armar e confirmar `Load + Serve`. O arquivo é novamente verificado por tamanho e SHA-256 imediatamente antes do processo IServer.
8. A barra não representa porcentagem de bytes: o IServer/LinkIO atual não expõe progresso transferido byte a byte à GUI. Console e eventos identificam apenas mensagens efetivamente recebidas.
9. Console usa Listbox com rolagem vertical/horizontal (evita o ciclo de desenho do `tk.Text` em Python 3.9/Tk 8.5 neste Mac); limpar e salvar TXT continuam disponíveis.
10. Exportar snapshot gera TXT.

## Preparação do IServer / LinkIO

O backend iserver/linkusb_linkio.c implementa a interface LinkIO usada pelo LOPS original. O recurso local é linkusb0. O build altera uma cópia de linkops.c para selecionar diretamente esse recurso, sem conexão via database. O runner Python prepara a linha original:

    iserver-linkusb -sc <arquivo.btl> -ss -sl linkusb0

-sc carrega o BTL e -ss executa o Serve original. A GUI prepara essa linha somente após o gate F297 e confirmação explícita; nenhum boot físico foi executado nesta etapa. O IServer não chama ResetLink nessa linha. O T425 deverá estar em WAIT_CONTROL antes do gate; o estado LOAD registrado em F294 bloqueia o envio até uma recuperação externa autorizada e nova consulta F271?.

Python e IServer compartilham o lock exclusivo /tmp/t425-sim-linkusb0.lock; um processo não abre LINKUSB enquanto o outro possui a sessão. O runner chama Load/Serve em subprocesso, encaminha stdout/stderr por threads e pode encerrar o processo host. Isso não implementa parada do guest.

## Testes sem hardware

tests/run_tests.sh testa GUI abre/fecha, dados BTL e hash, parser B4, consulta CDC simulada, B4 sem Bulk I/O, worker sem conexão/transmissão automática, supervisor stdout/stderr, proteção de execução e LinkIO C com leituras/escritas parciais, timeout e erro usando callbacks de teste.

Nenhum teste envia bytes para uma placa. A execução Hello World física não foi feita.

## F299.1 — Console gráfico OCCAM/IServer

Esta variante mantém a F297 intacta e adiciona a aba **Console OCCAM (IServer)**. O console usa `tk.Listbox` com tema escuro, fonte monoespaçada, barras vertical/horizontal, seleção, copiar e salvar TXT; `tk.Text` não é usado. stdout e stderr são drenados por leitores separados em threads. A GUI recebe eventos por fila e atualiza apenas na thread Tk. Cada evento registra fluxo, offset e bytes originais em Base64 no JSONL F299.1; texto UTF-8 é exibido incrementalmente, incluindo linhas ainda sem newline. A ordem é preservada dentro de stdout e dentro de stderr; não se afirma uma ordem global entre eles.

O console mostra apenas texto obtido dos pipes reais do subprocesso IServer. Rótulos de fluxo são metadados visuais. Início do processo, contadores de transporte, estado `RUNNING` ou código de saída não são apresentados como prova isolada de boot concluído ou sucesso OCCAM. A atribuição de texto stdout à saída do programa não é inferida automaticamente.

Resumo separado: Transporte, Bootstrap/boot, IServer e Saída do programa. O resumo usa linguagem de estado observado e não fabrica percentual de carregamento. Run, Stop e Reset seguem desabilitados. O gate F297 (F271 fresco em WAIT_CONTROL, B4 Link0 fresco/ocioso, armação e confirmação explícita) permanece aplicado. A tabela dos seis links e as ações de snapshot ficam na aba Snapshots para deixar o console amplo. A geometria se ajusta à tela e reserva margem inferior ao Dock do macOS.

### Ambiente Python/Tk

Na máquina validada, use Python 3.14 com Tk 9.1:

    cd ~/T425_OUTPUT/F299_1/control_center
    ./install_macos.sh
    ./run_macos.sh

O ambiente virtual fica em `.venv314`; o Python global não é alterado. Testes:

    ./tests/run_tests.sh

Os testes GUI e de subprocesso usam saída sintética identificada como fixture e não acessam USB nem iniciam boot físico.
