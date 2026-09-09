# T425-SIM v2.23 — retransmission cache

A v2.23 fecha a lacuna identificada na v2.22: o Reliable Link agora mantém uma cópia do pacote pendente.

## Estado

```text
IDLE
 │ begin()
 ▼
WAIT_ACK
 │
 ├── ACK(seq correto) ──► IDLE
 │
 └── timeout ──► RETRY
                   │
                   ▼
             recuperar PENDING
                   │
                   ▼
               retransmitir
                   │
                   ▼
                WAIT_ACK
```

O pacote completo, e não somente o número de sequência, é armazenado.

Assim uma retransmissão conserva:
- DEST;
- SOURCE;
- TTL;
- SEQ;
- LENGTH;
- PAYLOAD.

## Testes

O teste verifica:
1. criação do pending frame;
2. timeout;
3. recuperação do frame;
4. retransmissão com a mesma SEQ;
5. ACK correto encerrando a transação;
6. ACK de sequência incorreta sendo rejeitado;
7. falha após o número máximo de tentativas.
