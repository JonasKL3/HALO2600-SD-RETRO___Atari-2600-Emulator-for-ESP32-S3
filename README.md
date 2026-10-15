# HALO 2600 RETRO — SD Edition

**Atari 2600 no ESP32-S3 com biblioteca de ROMs exclusiva do cartão microSD integrado ao módulo TFT ILI9341.**

> **Estado: firmware experimental (V5 / pré-lançamento).** Este repositório é uma variante independente, derivada do projeto [HALO2600-RETRO](https://github.com/JonasKL3/HALO2600-RETRO___Atari-2600-Emulator-for-ESP32-S3). A V5 passou por revisão de código e simulações, **mas a execução completa no hardware ainda não foi confirmada**. Não confundir com a versão v1.0.0 estável do projeto original, que usava Wi-Fi/HTTPS/LittleFS.

## Destaques

- **ROMs exclusivamente do microSD:** sem Wi-Fi, downloads HTTPS ou LittleFS.
- Aceita arquivos `.a26` e `.bin` diretamente de `/Atari2600` (somente arquivos na pasta, sem subpastas).
- Indexa no máximo as primeiras **500 ROMs** da ordem retornada pelo cartão, depois ordena esse subconjunto alfabeticamente.
- Mostra incompatibilidade de tamanho e não tenta executar arquivos acima do limite do núcleo.
- Display **ILI9341 2,8"** e slot SD **compartilham a mesma instância HSPI** da `TFT_eSPI`, com CS independentes.
- Usa memória PSRAM para índice e framebuffer, controles Bluetooth com DabbleESP32 e retorna ao menu com START+SELECT.
- Diagnóstico detalhado no Serial em 115200 baud para inicialização do TFT, microSD, biblioteca e memória.

## Hardware e pinos

| Sinal | ESP32-S3 |
| --- | --- |
| TFT CS / DC / RST | GPIO10 / GPIO9 / GPIO4 |
| SPI SCK / MOSI / MISO (**TFT + SD**) | GPIO12 / GPIO11 / GPIO13 |
| SD CS | **GPIO14** |
| TFT LED/BL | 3V3 |

No **`User_Setup.h` ativo** da biblioteca `TFT_eSPI`: manter `USE_HSPI_PORT` e **remover `TOUCH_CS 14`**; o GPIO14 pertence exclusivamente ao microSD. Consulte [configuração física](docs/HARDWARE.md). O arquivo `TFT_eSPI_User_Setup_HALO2600_V5.h` dentro do firmware é apenas uma **referência**, não configura automaticamente a biblioteca.

## Instalação rápida

1. No Arduino IDE, selecione **ESP32S3 Dev Module**, **16 MB Flash**, **PSRAM habilitada** e **USB CDC On Boot habilitado**. Referência validada anteriormente: Arduino-ESP32 core **3.3.12**.
2. Instale `TFT_eSPI` e `DabbleESP32`, e confira a correção externa de memória do Dabble quando aplicável.
3. Configure a `TFT_eSPI` conforme [HARDWARE.md](docs/HARDWARE.md). **Não remova `USE_HSPI_PORT`.**
4. Insira microSD com pasta `/Atari2600` e arquivos `.a26` ou `.bin` que você tenha autorização para usar.
5. Abra [`HALO2600_SD_V5_HSPI_DIAG.ino`](firmware/HALO2600_SD_V5_HSPI_DIAG/HALO2600_SD_V5_HSPI_DIAG.ino) após **extrair todos os arquivos do ZIP**; nunca abra o `.ino` de dentro do WinRAR.
6. Compile, grave e abra o Monitor Serial em **115200 baud**. Verifique os logs de inicialização e o teste alternado TFT/SD.

Veja o [passo a passo completo](docs/INSTALLATION.md).

## Controles (Dabble GamePad)

| Botão | Menu | Durante o jogo |
| --- | --- | --- |
| D-pad cima/baixo | Navega um jogo | Movimento |
| D-pad esquerda/direita | Pula 8 jogos | Movimento |
| Analógico | Navega | Movimento |
| CROSS / SQUARE | Seleciona | FIRE |
| CIRCLE / START | — | RESET Atari |
| TRIANGLE / SELECT | — | SELECT Atari |
| START + SELECT (~350 ms) | — | Volta ao menu |

## Compatibilidade atual

O núcleo atual carrega arquivos de **2 KB a 16 KB**, com seleção base de mapper por tamanho (2/4K sem troca de bancos; 8K F8; 12K FA; 16K F6). A compatibilidade não é universal: ROMs podem exigir mappers diferentes apesar de terem o tamanho permitido. **32 KB e 64 KB ainda são planos, não recursos desta versão.** Veja [compatibilidade](docs/ROM_COMPATIBILITY.md).

## Estrutura

```text
HALO2600-SD-RETRO-ESP32S3/
├── firmware/HALO2600_SD_V5_HSPI_DIAG/    # sketch e núcleo Atari 2600
├── docs/                                  # hardware, instalação, arquitetura, testes
├── licenses/                              # referências de licenciamento
├── references/                            # origem dos componentes
├── release/                               # texto sugerido para uma pré-release
├── README.md
├── CHANGELOG.md
├── CREDITS.md
├── THIRD_PARTY_LICENSES.md
├── LICENSE-NOTICE.md
├── VERSION
└── .gitignore
```

## Estado dos testes

A V5 inclui resultados de verificações estáticas e testes com mocks (`firmware/.../TEST_REPORT_V5.txt`). **Não houve nesta entrega compilação com toda a toolchain Arduino ESP32 real nem certificação de funcionamento físico em ILI9341 + SD + Bluetooth.** Consulte [relatório consolidado](docs/TESTES_E_LIMITACOES.md). Os testes anteriores do cartão SD **isolado** foram concluídos pelo autor; o conjunto integrado ainda requer validação.

## Créditos e licenças

O núcleo deriva da linhagem **x2600 / Virtual 2600 / MCUME-espMCUME** e preserva arquivos de terceiros. `Tiasound.c` inclui o simulador de som TIA de **Ron Fries**. Dependências externas: **TFT_eSPI** e **DabbleESP32**. Leia [CREDITS.md](CREDITS.md), [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md) e [LICENSE-NOTICE.md](LICENSE-NOTICE.md); **não atribua licença MIT a todo o repositório**.

Este repositório **não inclui ROMs**. Cada jogo pertence ao seu respectivo titular. Projeto independente, sem associação ou endosso das marcas citadas.
