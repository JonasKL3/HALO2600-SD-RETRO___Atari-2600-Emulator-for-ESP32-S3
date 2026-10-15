# Diagnóstico de boot, TFT + SD

## Sintoma investigado

Na V3 experimental anterior, o Serial registrou `SD.begin()=TRUE`, abriu `/` e `/Atari2600`, depois ocorreu reinício quando o código chamou `tft.init()`. Isso aconteceu com instâncias SPI independentes usando o mesmo periférico HSPI.

## Correção planejada na V5

- Manter a configuração `USE_HSPI_PORT` previamente validada com TFT_eSPI.
- Remover o conflito `TOUCH_CS 14` com o novo `SD_CS 14`.
- Inicializar TFT primeiro e montar SD com `SD.begin(SD_CS, tft.getSPIinstance(), 1000000)`.
- Alternar desenhos TFT e leituras do SD quatro vezes na inicialização para detectar falhas de compartilhamento.

## O que copiar do Serial

1. CPU e estado do heap/PSRAM.
2. Etapa TFT: mensagem antes de `tft.init()` e retorno, se houver.
3. Etapa SD: tempo e resultado de `SD.begin`, tipo/capacidade, teste `/` e `/Atari2600`.
4. Etapa de alternância TFT/SD; indicar última rodada bem-sucedida.
5. Scanner e tempo até as primeiras 500 ROMs.
6. Etapa Dabble e seleção de um jogo simples de 4 KB.

A confirmação final depende de execução real na placa; testes simulados não certificam a coexistência elétrica nem a versão instalada da TFT_eSPI.
