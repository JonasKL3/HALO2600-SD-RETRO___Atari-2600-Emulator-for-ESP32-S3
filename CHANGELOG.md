# Changelog

## [0.5.0-alpha.1] — HALO2600 SD Edition / firmware de teste V5 (2026-10-15)

### Added
- microSD como **fonte exclusiva** de ROMs (`/Atari2600`, `.a26` e `.bin`).
- Varredura limitada às primeiras **500 ROMs elegíveis** na ordem do FAT; ordenação alfabética posterior do conjunto.
- Menu paginado, indicador de ROM fora do tamanho aceito pelo núcleo (2–16 KB).
- Diagnóstico de inicialização no Serial: heap/PSRAM, TFT, SD, acesso aos diretórios, leitura de ROM e alternância SPI.

### Changed
- Núcleo Atari e controles Dabble reaproveitados do HALO2600; removido fluxo Wi-Fi/HTTPS/LittleFS no firmware SD.
- Compartilhamento SPI: `SD.begin(14, tft.getSPIinstance(), 1000000)` mantendo `USE_HSPI_PORT`.
- GPIO14 reservado ao SD; `TOUCH_CS 14` proibido pelo firmware.

### Validation
- Verificações sintáticas do núcleo e simulação do scanner e do fluxo do emulador documentadas no relatório interno V5.
- Inicialização física com TFT + SD + Dabble e compilação integral Arduino-ESP32 ainda **pendentes**.

### Not yet implemented
- Compatibilidade de ROMs 32 KB e mappers F4/F4SC, depois 64 KB.
- Índice persistente no SD, busca e capas de jogos.
