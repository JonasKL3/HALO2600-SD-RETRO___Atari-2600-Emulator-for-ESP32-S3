# Instalação (Arduino IDE)

1. Faça download do ZIP e **extraia todos os arquivos** antes de abrir o `.ino` (nunca execute pelo WinRAR).
2. Selecione **ESP32S3 Dev Module** e Arduino-ESP32 core 3.3.12, que é a configuração de referência do projeto anterior.
3. Ajuste **Flash Size 16MB (128Mb)**, **PSRAM Enabled**, **USB CDC On Boot Enabled**, CPU 240 MHz e **Partition Scheme Huge APP (3MB No OTA/1MB SPIFFS)** ou equivalente com APP >=3MB (mesmo que LittleFS não seja usado).
4. Instale as bibliotecas `TFT_eSPI` e `DabbleESP32` e configure `TFT_eSPI` conforme `HARDWARE.md`.
5. Especialmente: **mantenha `USE_HSPI_PORT`** e comente/remova `#define TOUCH_CS 14` no setup ativo.
6. Formate o SD com sistema de arquivos suportado (`FAT32` recomendado). Faça backup antes de formatar; este firmware apenas lê o cartão.
7. Crie no cartão `/Atari2600` e coloque seus arquivos `.a26/.bin` diretamente nessa pasta.
8. Abra `firmware/HALO2600_SD_V5_HSPI_DIAG/HALO2600_SD_V5_HSPI_DIAG.ino` no Arduino IDE e compile/grave.
9. Abra Monitor Serial em **115200 baud**; confira as etapas: memória, TFT, SD, alternância TFT/SD, scanner, Dabble e menu.
10. Se a placa reiniciar, envie o log completo para investigação (veja `SPI_DIAGNOSTICS.md`).

**Nota:** `TOUCH_CS` pode estar definido num arquivo de `User_Setups`, não somente em `User_Setup.h`; o ajuste precisa ser feito no setup **realmente compilado**.

**Nota:** a leitura do SD isolado pelo usuário funcionou, mas a integração completa da V5 ainda precisa de validação física. Não publique como release estável sem testar.
