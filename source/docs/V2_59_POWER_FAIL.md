# V2.59 — teste de falha de energia

O teste físico deverá interromper a alimentação durante diferentes fases:

1. antes de apagar o slot antigo;
2. durante a escrita do novo slot;
3. após a escrita mas antes da ativação;
4. após a ativação.

Critério:

> Deve existir sempre pelo menos uma configuração válida recuperável, exceto
> quando o sistema já estava sem qualquer configuração válida antes do teste.

O mecanismo de dois slots é a base para isso.
