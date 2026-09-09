# Matriz de testes v1.5

| Teste | Esperado |
|---|---|
| DATA + ACK | waiting_ack → false |
| ACK errado | permanece pending |
| timeout 1 | retransmission=1 |
| timeout 2 | retransmission=2 |
| timeout 3 | retransmission=3 |
| timeout final | timeouts=1 |
| payload vazio | permitido |
| payload > 240 | rejeitado |

Após isso: testes físicos com perda artificial de bytes e CRC.
