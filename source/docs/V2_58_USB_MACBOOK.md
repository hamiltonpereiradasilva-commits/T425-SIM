# V2.58 — relação com o MacBook

O MacBook será o host do console.

A arquitetura prevista é:

```text
MacBook
   |
 USB
   |
Pico USB CDC
   |
Config CLI
   |
Node Config
```

O terminal do MacBook poderá enviar comandos ASCII.

Nesta etapa não é criado ainda um aplicativo gráfico para macOS. O objetivo é
primeiro estabilizar o protocolo de configuração, pois um terminal simples é
mais fácil de depurar e automatizar.

Depois disso poderá ser criada uma ferramenta específica para macOS que esconda
os comandos e apresente a configuração em uma interface gráfica.
