# Compatibilidade Atari 2600 — estado experimental V5

Tamanhos aceitos atualmente para execução: **2 KB a 16 KB**. Arquivos maiores podem ser indexados e mostrados com `!` no menu, mas são recusados quando selecionados.

| ROM | Detecção padrão do núcleo atual | Observações |
| --- | --- | --- |
| 2 KB | duplicada para 4 KB, sem bankswitch | formato simples |
| 4 KB | sem bankswitch | tamanho comum |
| 8 KB | F8 | outros mappers de 8 KB, como E0, podem exigir configuração distinta |
| 12 KB | FA | cartuchos especiais exigem RAM e lógica correta |
| 16 KB | F6 | F6SC e outros não detectados universalmente |
| 32 KB ou mais | não suportado | implementação futura — requer mappers, não só aumento de buffer |

A extensão `.a26` ou `.bin` **não define o mapper**, nem garante que os bytes sejam uma ROM válida. Um arquivo indexado pode carregar e não executar corretamente. Para avançar a 32 KB serão necessários mappers como F4, testes de bankswitch e regressões das ROMs menores.
