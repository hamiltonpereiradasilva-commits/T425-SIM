# V2.68 — matriz de testes

| Cenário | Resultado esperado |
|---|---|
| A válida, B inválida | A ativa |
| B válida, A inválida | B ativa |
| A e B válidas | maior generation ativa |
| target erase falha | ativo permanece |
| program falha | ativo permanece |
| read-back inválido | novo slot rejeitado |
| CRC inválido | novo slot rejeitado |
| commit válido | target torna-se ativo |
| reboot após commit | nova geração recuperada |
| mesmo UF2 em P00/P01 | identidade vem da Flash |

Os testes físicos de falha de energia ficam para a etapa seguinte.
