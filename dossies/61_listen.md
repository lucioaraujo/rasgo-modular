# LISTEN — escuta por regiões

Implementação local de 17/10/2026, posterior à release 0.1.6.
Fonte: [Listen.hpp](../src/dsp/Listen.hpp). Família OUT; 16 HP.
Licença autoral: AGPL-3.0-or-later.

O sinal que chega pode provocar ações distintas conforme o que acontece nos
graves, médios e agudos. Há três níveis contínuos e três pulsos de ataque,
além da passagem direta de áudio THRU. As bandas não isolam instrumentos.
Um bumbo com estalo pode estimular mais de uma banda.

## Contrato

IN recebe áudio. THRU preserva cada amostra finita de cada canal, sem atraso
adicional; valores não finitos viram zero. A análise usa magnitude média dos
canais após os filtros, evitando o cancelamento de um estéreo em oposição.
Os estados estão dimensionados para os oito canais máximos do AudioBlock.

LOW/MID/HIGH: envelopes lineares, limitados a 0–1; sem normalização,
compensação de ganho ou identificação de fonte. L-HIT/M-HIT/H-HIT: pulsos de
5 ms. Não há estimativa de BPM nem clock interpolado entre ataques.

Os cortes padrão são 200 Hz e 2 kHz. Baixas: Butterworth de segunda ordem
passa-baixas; médias: passa-altas no primeiro corte, seguido de passa-baixas
no segundo; altas: passa-altas no segundo. A topologia é causal e as bandas
se sobrepõem. Não se promete soma complementar nem fase zero.

Níveis: ataque de 1–81 ms e queda de 40–1240 ms, pelo RESP. Ataques usam
seguidores próprios: rápido 0,8/30 ms e lento 25/100 ms. Disparam quando o
rápido supera o lento por uma razão de 1,45–3,0 mais o piso ajustado, com
rearme independente de 20–250 ms. FLOOR vai de −72 a −18 dBFS. A latência
de detecção depende da frequência, amplitude e contraste do ataque; não é
igual à duração do pulso. Depois de mudar um corte há inibição de ataques
por 50 ms para não transformar o ajuste num disparo.
Modulação contínua dos cortes pode manter essa inibição: os níveis continuam
respondendo, mas os ataques esperam o corte estabilizar. Aprimorar essa
transição para modulações contínuas é trabalho posterior.

Filtros e envelopes são reiniciados em prepare(). Coeficientes mudam por
bloco; estados e saídas são pré-alocados. O processamento não faz I/O,
bloqueios ou alocações. A calibração do LISTEN fica fora da VARIA; modulações
explícitas por cabo continuam disponíveis.

## Proveniência e limites

Referência conceitual: escuta por regiões do Trioio, commit
`050637f8baf6ff1545965b7e6b0769247d8c201b`, BandSplit e LINHAS.md, já
registrados nas tarefas de 13/10. Nenhum arquivo daquele instrumento foi
copiado. A implementação causal é própria: não usa a filtragem offline de
ida e volta, o acervo, a normalização ou os descritores globais do Trioio.
O SCOPE e os índices de suas portas permanecem intactos.

Teste: [test_reactive_modules.cpp](../tests/test_reactive_modules.cpp), incluindo
bandas, ataques, silêncio, estéreo, taxas de amostragem, partição de blocos,
reinício, valores inválidos, heap e um patch reativo com voz sintetizada.
Aceite auditivo do autor e integração de pulso/croma ficam para outra etapa.
