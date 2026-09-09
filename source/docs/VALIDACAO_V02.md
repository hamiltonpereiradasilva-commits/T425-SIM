# Validação v0.2

Os três testes de fumaça incorporados ao firmware reproduzem exemplos de codificação do manual T425:

1. PFIX 3 + LDC 5 -> A = 0x35.
2. NFIX 1 + LDC 1 -> A = 0xFFFFFFE1 (-31).
3. LDC 10 + LDC 20 + ADD -> A = 30.

O manual informa que PFIX/NFIX estendem operandos por nibbles e apresenta explicitamente os dois primeiros exemplos. A tabela 11.9 define as funções primárias e as tabelas 11.11–11.18 definem operações aritméticas, controle, I/O e scheduling.
