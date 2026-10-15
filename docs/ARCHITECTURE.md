# Arquitetura — HALO2600 SD Edition

```text
ESP32-S3 N16R8
 ├── TFT_eSPI (ILI9341, host HSPI)
 │      └── getSPIinstance() ─────┐
 ├── SD (CS GPIO14) <──────────────┘   compartilhamento de barramento
 │      └── /Atari2600/*.a26, *.bin
 │             └── scanner limitado a 500 ROMs
 │                    └── menu paginado (TFT)
 │                           └── seleção -> emu_LoadFile -> theCart -> vcs_Start
 ├── PSRAM: framebuffer e índice da biblioteca
 └── DabbleESP32 GamePad: menu / FIRE / RESET / SELECT
```

O emulador foi reaproveitado do HALO2600 v1.0.0. A camada `emuapi.cpp` usa `SD.open()` para carregar o **conteúdo** da ROM de forma pontual; não há leitura da ROM em cada frame do jogo após o carregamento em memória. A mídia **não é modificada**, copiada para Flash nem apagada ao sair do jogo.

## SPI compartilhado

O slot microSD faz parte do mesmo **módulo físico** do ILI9341, mas tem seu próprio circuito seletor no barramento. O código usa a instância criada pelo TFT_eSPI, e **não** dois objetos HSPI independentes. Isso evita reproduzir os resets da V3 em que o SD funcionou e `tft.init()` reiniciou a placa.

## Índice e memória

A V5 indexa até 500 arquivos correspondentes à extensão e para a leitura do diretório tão logo encontre esse número. Os 500 são os primeiros que o sistema de arquivos entrega, **não necessariamente os 500 alfabeticamente iniciais**. A ordenação acontece depois, apenas nesse subconjunto. Entrada de pastas secundárias não é recursiva. Nomes/caminhos que excedem a capacidade da entrada podem ser ignorados para evitar truncamentos inválidos.

## Futuros mappers

Aumento de compatibilidade de cartuchos de 32 KB ou mais dependerá de ampliar o buffer do cartucho e implementar a lógica específica de bankswitch; **não está incluído** na V5.
