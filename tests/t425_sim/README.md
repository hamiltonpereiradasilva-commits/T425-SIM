# Aceitação T425-SIM

Use `t425-sim` nas quatro etapas completas e `t425-sim-test` somente no smoke, com observer de teste independente dos módulos produtivos.
`python3 tests/t425_sim/regression.py --executable build/t425_sim/t425-sim --smoke-executable build/t425_sim/t425-sim-test --fixtures CAMINHO_DA_TOOLCHAIN_T04 --output CAMINHO_NOVO_DE_RESULTADOS`

As entradas ILINK.BTL, OCCONF.BTL, ICOLLECT.BTL, test04.tah, hello.pgm, hostio.inc e bibliotecas guest devem ser fornecidas explicitamente. São ativos da toolchain INMOS, não objetos do build host. O teste não contém caminhos de instalações temporárias.
O perfil é IBOARDSIZE=#400000, T425SIM0 e compat-v14. Os hashes T04 de referência F245 estão no script. Não são executados T05/T06.
Retorno zero de ReadLink não é falha. Há um limite total de900 segundos por estágio; o supervisor da validação F259 também registra progresso e estado antes de interrupção externa.
