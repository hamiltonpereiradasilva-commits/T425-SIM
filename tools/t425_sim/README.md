# T425-SIM com IServer INMOS

Build: `python3 tools/t425_sim/build.py`. Requer GCC, Python 3, libusb-1.0 e seus headers.
O build deriva os caminhos da raiz do projeto; não usa snapshots, objetos herdados ou diretórios de evidência.
O executável `build/t425_sim/t425-sim` contém o runtime e os handlers INMOS originais, no perfil aprovado `compat-v14`.
O target adicional `t425-sim-test` inclui somente observação de Boot/HOSTIO para regressão.

Execução: `IBOARDSIZE=#400000 build/t425_sim/t425-sim -sz -sl T425SIM0 -sb ILINK.BTL ...`.
As entradas INMOS são fornecidas pelo usuário no cwd. A árvore não incorpora bibliotecas guest históricas em arquivos de fonte do host.

A integração preserva a base microprogramada F253, seu coordinator terminal, o contrato F248 enqueue antes de ACK, o relógio virtual e o progresso após SendReply.
A instrumentação histórica foi removida nos pontos de chamada, sem criar stubs ou símbolos fracos.
T05/T06 do runner R2 são evidência histórica F257, não aprovação deste candidato.
