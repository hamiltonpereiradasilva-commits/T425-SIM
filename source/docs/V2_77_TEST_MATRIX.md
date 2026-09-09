# V2.77 — matriz de testes

| Teste | Resultado esperado |
|---|---|
| Boot com A/B vazios | DEFAULT |
| SAVE inicial | A/generation 1 |
| LOAD | mesma configuração |
| SAVE novamente | B/generation 2 |
| SAVE novamente | A/generation 3 |
| CLEAR | A/B inválidos |
| CRC corrompido em A | B ainda pode ser selecionado |
| CRC corrompido no target após program | COMMIT falha |
| Erase/program timeout | COMMIT falha |
| Mesmo UF2 em P00/P01 | configurações independentes |
| Reboot após SAVE | configuração permanece |
| Power-loss antes do commit | slot anterior permanece válido |
| Power-loss após commit | pelo menos um slot válido |

O teste de power-loss precisa ser executado em hardware.
