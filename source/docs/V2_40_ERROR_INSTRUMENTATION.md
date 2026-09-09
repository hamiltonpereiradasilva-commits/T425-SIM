# T425-SIM v2.40 — instrumentação de erros

## Objetivo

Fechar a lacuna da v2.39: distinguir explicitamente perda de sincronismo, erro de formato e CRC inválido.

O parser agora conta:

- `frames_ok`
- `crc_errors`
- `format_errors`
- `sync_losses`

A telemetria do nó acrescenta:

```text
CRC=
FORMAT=
SYNCLOSS=
INJECT=
```

## Injeção deliberada

No P00, cada 100º frame novo tem o CRC deliberadamente alterado.

A finalidade é produzir uma falha conhecida e verificar se:

1. P01 rejeita o frame;
2. P00 não recebe ACK;
3. ocorre timeout;
4. ocorre retransmissão;
5. os contadores registram o evento.

## Importante

Esta é uma falha de teste deliberada. Em uma implementação final, o mecanismo de corrupção será compilado/desativado por configuração de bancada.

## Critério

Depois de aproximadamente 100 frames, deve existir pelo menos uma injeção.

O comportamento esperado é:

```text
INJECT  > 0
CRC     > 0   no receptor
RETX    > 0   no transmissor
```

`TIMEOUT` poderá permanecer zero se o frame retransmitido for recebido corretamente. Nesta versão, a retransmissão mantém a corrupção deliberada para forçar a trajetória de timeout; isso é intencional para validar o limite de tentativas.
