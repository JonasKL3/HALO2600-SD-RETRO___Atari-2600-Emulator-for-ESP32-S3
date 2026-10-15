# Publicação no GitHub — sem alterar o HALO2600 original

## Repositório recomendado

Nome sugerido: `HALO2600-SD-RETRO___Atari-2600-Emulator-for-ESP32-S3`

Descrição sugerida:

> Experimental Atari 2600 emulator for ESP32-S3 with ILI9341 TFT, shared HSPI microSD ROM library and Dabble Bluetooth controls.

**Importante:** o HALO2600 original continua como baseline v1.0.0 da versão Wi-Fi/HTTPS. **Não substitua sua branch `main` nem a release original com este firmware experimental.**

## Como publicar

1. No GitHub, use **New repository** (público, se desejar). Não gere README automático porque o pacote já contém um.
2. Envie todo o **conteúdo desta pasta raiz** para o novo repositório (README.md, firmware/, docs/, licenses/, references/, etc.). Não faça upload do ZIP como se ele fosse o conteúdo do repositório: extraia antes.
3. Confira a estrutura e a presença do arquivo `HALO2600_SD_V5_HSPI_DIAG.ino` na pasta homônima.
4. Mensagem de commit sugerida: `Initial SD Edition experimental firmware (V5 shared HSPI)`.
5. Se desejar criar uma release, crie uma **pre-release**, com tag `v0.5.0-alpha.1`, título `HALO2600 SD Edition — V5 Experimental (shared HSPI)`. **Não** marque como `Latest`/estável até validar na placa.
6. Copie o texto de `release/RELEASE_NOTES_v0.5.0-alpha.1.md` para a descrição da pré-release e anexe o ZIP do código fonte, se preferir.

## Credenciais, ROMs e licenças

- Não versione `.a26`, `.bin`, dumps de ROMs nem arquivos privados.
- Não poste senhas Wi-Fi ou dados pessoais; SD Edition não usa Wi-Fi.
- Preserve os avisos de terceiros dentro de `.c/.h`, além dos documentos de crédito e licença.
- Ao mudar arquivos, atualize a lista e checksums antes de publicar uma nova versão.
