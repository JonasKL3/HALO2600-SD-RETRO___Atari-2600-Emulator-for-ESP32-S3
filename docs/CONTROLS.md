# Controles pelo aplicativo Dabble

Conexão Bluetooth anunciada pelo firmware: `ESP32-S3-HALO2600-SD`.

| Dabble | Menu | Jogo Atari 2600 |
| --- | --- | --- |
| Cima / baixo | próximo/anterior jogo | direcional |
| Esquerda / direita | salta 8 posições | direcional |
| Analógico | navegação | direcional |
| CROSS / SQUARE | iniciar jogo | FIRE |
| CIRCLE / START | — | RESET |
| TRIANGLE / SELECT | — | SELECT |
| START+SELECT por ~350 ms | — | voltar ao menu |

A implementação preserva a inversão interna horizontal e a prioridade do D-pad sobre o analógico validadas no HALO2600 original.
