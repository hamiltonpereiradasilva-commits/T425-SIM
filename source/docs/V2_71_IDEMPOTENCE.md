# V2.71 — comportamento de sessão

Após COMMIT bem-sucedido, o staging é limpo.

Isso impede que uma segunda chamada acidental reutilize silenciosamente a
mesma configuração staged.

Se COMMIT falhar, o staging não é limpo, permitindo diagnóstico/repetição sem
inventar uma nova configuração.
