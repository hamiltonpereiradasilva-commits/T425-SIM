# V2.62 — critérios de aceitação

- [x] região persistente de 8192 bytes definida;
- [x] dois slots de 4096 bytes;
- [x] região derivada do fim da Flash do aplicativo;
- [x] símbolos de linker para início/fim;
- [x] validação de tamanho e alinhamento;
- [x] separação entre contrato e linker script oficial;
- [ ] integrar fragmento no linker real do projeto;
- [ ] confirmar FLASH_ORIGIN/LENGTH da placa;
- [ ] verificar tamanho máximo da imagem;
- [ ] implementar erase/program físico;
- [ ] read-back;
- [ ] power-loss.
