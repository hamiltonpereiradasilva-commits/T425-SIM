# Exemplos de teste

O primeiro teste deve ser feito sem a rede física completa:

1. Grave `t425_pico0.uf2` no Pico 0.
2. Abra o monitor serial USB.
3. Confirme a identificação `NODE 0`.
4. Repita para Picos 1, 2 e 3.
5. Em seguida, conecte um transceptor RS-485 ao UART de gerenciamento.

Para testar a VM, uma imagem mínima pode ser colocada na RAM a partir de
`0x80000070`. A codificação segue a organização de nibbles do T425
(PFIX/NFIX + função), conforme o manual do T425.

Importante: esta primeira entrega implementa uma **VM T425 experimental**.
Ela não declara compatibilidade binária completa com o IMS T425. Os opcodes
ainda não implementados deliberadamente entram em estado de erro/halt.
