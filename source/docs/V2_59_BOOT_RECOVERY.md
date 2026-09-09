# V2.59 — boot e recuperação

O boot deve executar:

```text
read SLOT A
read SLOT B
validate A
validate B
select highest generation valid
```

Se nenhum slot for válido:

```text
RECOVERY MODE
```

No recovery, o nó não deve iniciar uma configuração de rede potencialmente
corrompida.

Ele deverá permanecer disponível para reconfiguração pelo USB.

Isso cria o caminho:

```text
configuração inválida
       ↓
RECOVERY
       ↓
USB
       ↓
nova configuração
       ↓
commit
       ↓
boot normal
```
