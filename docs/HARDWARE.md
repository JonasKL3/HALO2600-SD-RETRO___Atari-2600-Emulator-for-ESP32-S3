# Hardware e pinagem

**Placa:** ESP32-S3 DEV MODULE-1 N16R8 (16 MB Flash, 8 MB PSRAM). 
**Display:** módulo TFT 2,8" ILI9341 (240x320) SPI, com **slot microSD integrado fisicamente**; no barramento ele opera como dispositivo SPI independente, com CS próprio.

| Dispositivo / sinal | Pino |
| --- | --- |
| TFT VCC e GND | 3V3 e GND |
| TFT CLK/SCK | GPIO12 |
| TFT MOSI/SDI | GPIO11 |
| TFT MISO/SDO | GPIO13 |
| TFT CS | GPIO10 |
| TFT DC/RS | GPIO9 |
| TFT RST | GPIO4 |
| TFT BL/LED | 3V3 |
| SD SCK | GPIO12 |
| SD MOSI | GPIO11 |
| SD MISO | GPIO13 |
| SD CS | **GPIO14** |

## Configuração TFT_eSPI

No `User_Setup.h` **ativo** (verifique `User_Setup_Select.h` caso outro setup esteja selecionado):

```cpp
#define ILI9341_DRIVER
#define TFT_MISO 13
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_CS   10
#define TFT_DC    9
#define TFT_RST   4
#define SPI_FREQUENCY 5000000
#define SPI_READ_FREQUENCY 10000000
#define USE_HSPI_PORT
// Não definir TOUCH_CS 14
```

O firmware contém `#error` se `USE_HSPI_PORT` ou pinos essenciais estiverem incorretos, ou se `TOUCH_CS=14` continuar definido. O touch não é usado na unidade investigada. **Não use dois objetos `SPIClass(HSPI)` independentes**: o TFT_eSPI inicializa o host, e a biblioteca SD recebe `tft.getSPIinstance()`.

O arquivo `firmware/HALO2600_SD_V5_HSPI_DIAG/TFT_eSPI_User_Setup_HALO2600_V5.h` é apenas **referência**: não configura o TFT_eSPI automaticamente.

`iopins.h` contém pinagem herdada de outro hardware e **não corresponde** à conexão real deste projeto.
