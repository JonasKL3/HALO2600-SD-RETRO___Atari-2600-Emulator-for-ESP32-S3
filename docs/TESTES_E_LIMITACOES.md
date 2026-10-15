# Testes e limitações — V5 experimental

## Evidências disponíveis

- **Cartão isolado na placa física (realizado pelo autor):** microSD SDHC de aproximadamente 14,8 GB montado por SPI a 1 MHz, com leitura de vários diretórios e arquivos; isto valida **somente o SD isolado**.
- **Falha do teste integrado V3 (real):** SD montou e abriu `/Atari2600`, mas a placa reiniciou durante `tft.init()` ao usar objetos SPI independentes sobre HSPI.
- **V5 (simulação/verificação estática):** o pacote original inclui `TEST_REPORT_V5.txt`, documentando testes de sintaxe do núcleo, mocks Arduino para `.ino`/`emuapi.cpp`, simulação de 900 ROMs com parada em 500, ordenação, leitura e transições de menu.

## Ainda não validados nesta entrega

- Compilação integral usando Arduino-ESP32 core real instalado.
- Boot completo da V5 e alternância TFT/SD na placa física.
- Pareamento Bluetooth Dabble junto ao SD/TFT.
- Execução prolongada de ROMs reais e compatibilidade dos mappers.

**Não rotular como `stable/latest` antes da confirmação desses itens.**

## Regressões previstas

1. ROM 4 KB conhecida (sem bankswitch) abre e executa sem tela preta.
2. ROM 8 KB F8 abre e executa.
3. ROM 16 KB F6 abre e executa.
4. Escolha e retorno ao menu por START+SELECT.
5. Diversas trocas de jogo sem reset e sem erro de memória.
6. Reiniciar com cartão com centenas de `.a26`; verificar limite 500.
