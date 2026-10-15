HALO2600 SD ONLY V5 - DIAGNOSTICO SPI + MICROSD INTEGRADO AO DISPLAY

IMPORTANTE: VERSAO EXPERIMENTAL. AINDA PRECISA TESTAR NA SUA PLACA.
A release v1.0.0 do GitHub nao foi alterada.

HARDWARE
 ESP32-S3 N16R8, ILI9341 SPI 2.8 polegadas, microSD no mesmo modulo:
 TFT MOSI=GPIO11 | MISO=GPIO13 | SCK=GPIO12 | CS=GPIO10 | DC=GPIO9 | RST=GPIO4
 microSD MOSI=GPIO11 | MISO=GPIO13 | SCK=GPIO12 | CS=GPIO14

ANTES DE COMPILAR (OBRIGATORIO)
 1. Abra o User_Setup.h ATIVO da biblioteca TFT_eSPI (ou User_Setups selecionado).
 2. MANTENHA #define USE_HSPI_PORT (nao remover).
 3. MANTENHA ILI9341_DRIVER e os pinos testados (ver arquivo de referencia).
 4. REMOVA OU COMENTE #define TOUCH_CS 14.
 5. Verifique que nao ha outra definicao TOUCH_CS para o GPIO14.
 6. O codigo .ino tem verificacoes #error para essas configuracoes.

COMO ABRIR
 Extraia o ZIP inteiro. Abra HALO2600_SD_V5_HSPI_DIAG.ino de dentro da
 pasta HALO2600_SD_V5_HSPI_DIAG (nao abra de dentro do WinRAR).
 Arduino IDE 2.x / ESP32S3 Dev Module / Arduino core ESP32 3.3.12
 Flash 16MB | PSRAM 8MB (habilitada) | USB CDC On Boot Enabled.
 Monitor Serial: 115200 baud.

CARTAO SD
 Pasta /Atari2600 (exatamente como voce informou)
 Arquivos .a26 ou .bin dentro dessa pasta (nao enumera subpastas).
 Indexa ate 500 ROMs encontradas na ordem FAT e ordena apenas as 500 indexadas.
 ROMs acima de 16KB sao mostradas mas nao executadas pelo nucleo atual.
 Nenhuma ROM e copiada ou apagada. Nenhuma pasta e criada; SD so e lido.

BOOT
 1. Preparar pinos CS (TFT e SD altos)
 2. tft.init() no HSPI original da TFT_eSPI
 3. SD.begin(SD_CS=14, tft.getSPIinstance(), 1MHz)
 4. Abrir raiz e pasta /Atari2600
 5. Alternar 4x desenho TFT e acesso SD
 6. Indexar ate 500 ROMs e ordenar
 7. Tentar abrir e ler primeiros 16 bytes da primeira ROM indexada
 8. Alternar TFT/SD novamente
 9. Dabble GamePad
10. Exibir menu e permitir executar a ROM diretamente do SD

DABBLE
 UP/DOWN = um jogo | LEFT/RIGHT = pular 8 | CROSS/SQUARE = jogar
 No jogo: CROSS/SQUARE FIRE; CIRCLE/START RESET; TRIANGLE/SELECT SELECT
 START+SELECT 350ms = voltar ao menu.

SEM WIFI, SEM HTTPS, SEM LITTLEFS. ROMS SOMENTE PELO SD.

OBSERVACOES
 * O arquivo iopins.h e legado do nucleo original, NAO representa a pinagem real.
 * As provas de compatibilidade de mapper ainda sao limitadas ao nucleo antigo.
 * O SD inicializa a 1MHz para favorecer estabilidade.
 * Copie o log Serial completo de boot, especialmente caso ocorra reinicio.
 * O bug de memoria conhecido do DabbleESP32 depende da biblioteca instalada e
   nao e resolvido por este firmware: consulte docs/DABBLE_BLE_RESET.md do GitHub.
 * ROMs comerciais nao estao incluidas neste pacote.
