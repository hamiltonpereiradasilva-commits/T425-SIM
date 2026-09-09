# V2.63 — verificação e power-fail

Após programar o slot inativo:

```text
read-back
   |
   v
MAGIC/VERSION/LENGTH
   |
   v
CRC32
   |
   v
generation
```

Somente se todos forem válidos o commit é aceito.

O teste de energia deverá interromper a alimentação durante:

- erase;
- program;
- read-back/commit.

Resultado esperado:

```text
novo slot válido  -> novo slot ativo
novo slot inválido -> slot anterior continua recuperável
```
