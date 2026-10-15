// TFT_eSPI - HALO2600 SD ONLY v5 - ESP32-S3 N16R8 + ILI9341 2.8in
// Copiar as definicoes para o setup ATIVO da biblioteca TFT_eSPI.
// NAO copiar este arquivo para a pasta do sketch com este nome esperando autodeteccao.
#define ILI9341_DRIVER
#define TFT_MISO 13
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_CS   10
#define TFT_DC    9
#define TFT_RST   4

// Touch nao usado. GPIO14 pertence exclusivamente ao microSD.
// #define TOUCH_CS 14  // PROIBIDO

#define SPI_FREQUENCY       5000000
#define SPI_READ_FREQUENCY 10000000
#define SPI_TOUCH_FREQUENCY 2500000
#define USE_HSPI_PORT
