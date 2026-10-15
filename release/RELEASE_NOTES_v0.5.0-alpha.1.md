# HALO2600 SD Edition — V5 Experimental (shared HSPI)

**Pré-release experimental.** Variante SD-only do HALO2600 para ESP32-S3 N16R8 e TFT ILI9341 com slot microSD integrado.

## Mudanças

- Carregamento de ROMs `.a26`/`.bin` **somente pelo microSD** na pasta `/Atari2600`.
- Biblioteca dinâmica com **até 500 ROMs** por inicialização.
- Compartilhamento HSPI corrigido usando `tft.getSPIinstance()` para TFT + SD, com CS 10 (TFT) e 14 (SD).
- Mantidos controles Dabble, framebuffer PSRAM e retorno START+SELECT.
- Diagnósticos de boot, cartão, índice, memória e alternância SPI no Monitor Serial.

## Requisitos

- Arduino-ESP32 core de referência: 3.3.12, board ESP32S3 Dev Module.
- `TFT_eSPI`: **USE_HSPI_PORT habilitado** e **TOUCH_CS 14 removido** no `User_Setup` ativo.
- Cartão microSD com a pasta `/Atari2600` e ROMs próprias/autorizadas.

## Limites conhecidos

- Núcleo aceita ROMs de 2 KB a 16 KB; suporte de 32 KB/F4 ainda não foi implementado.
- Compatibilidade de mapper não é universal.
- **Teste integrado físico do firmware V5 ainda pendente**. Relatório em `docs/TESTES_E_LIMITACOES.md`.

**Não inclui ROMs, nem altera a release estável v1.0.0 do HALO2600 original.**
