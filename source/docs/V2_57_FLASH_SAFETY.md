# V2.57 — segurança da configuração em Flash

A implementação futura deve usar pelo menos dois slots:

```text
FLASH
+-------------------+
| SLOT A            |
| config + CRC      |
+-------------------+
| SLOT B            |
| config + CRC      |
+-------------------+
```

Boot:

1. validar A;
2. validar B;
3. se ambos válidos, escolher o registro mais recente;
4. se somente um válido, usar o válido;
5. se nenhum válido, entrar em configuração/recovery.

Isso evita que uma interrupção durante a gravação deixe o nó sem configuração.

A v2.57 já fornece a unidade fundamental: registro versionado + CRC32.
