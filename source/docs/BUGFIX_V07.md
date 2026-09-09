# Correção final v0.7 — rendezvous

A validação encontrou uma incompatibilidade entre o teste legado do canal, que espera `CH_READY` após a segunda metade do rendezvous, e a primeira implementação, que liberava imediatamente para `CH_FREE`.

A solução final mantém `CH_READY` como estado de rendezvous concluído até que o processo que ficou bloqueado repita a operação. O processo que executa a segunda metade recebe `true` imediatamente; o primeiro processo, ao ser retomado, também recebe `true` e então o canal volta a `CH_FREE`.

Isso permite preservar a semântica do teste existente e, ao mesmo tempo, modelar o bloqueio/retomada necessário à execução concorrente.
