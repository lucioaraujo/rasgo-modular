# PHRASE — motivo interpretado por estímulos

Implementação local de 17/10/2026, posterior à release 0.1.6.
Fonte: [Phrase.hpp](../src/dsp/Phrase.hpp). Família DECISION; 20 HP.
Licença autoral: AGPL-3.0-or-later. Código original, sem tabelas ou código
copiados do Trioio ou de um sintetizador externo.

Cada CLK externo toca um passo de um motivo de 2–16 posições. A seed define
um percurso limitado, guardado em um array de 16 valores; variar a execução
não apaga esse motivo. O músico escolhe o âmbito, a direção, a dose de
variação, as pausas e a probabilidade de retornar à altura de apoio no final.

## Contrato

- PITCH de entrada: apoio em 1 V/oct, capturado no primeiro passo de cada
  frase; sem cabo, zero. Valores são limitados a ±6 oitavas.
- CLK: borda ascendente; sem relógio externo não há avanço autônomo.
- RST: borda ascendente reinicia percurso, RNG, altura e articulação; aguarda
  o próximo relógio. Um gate de reset mantido alto impede o avanço.
- STIM: intensidade 0–1, padrão 1 sem cabo. Aumenta acentos, reduz a
  probabilidade de pausa e amplia a variação autorizada pelo knob VAR.
- RET: solicita uma conclusão no próximo CLK: apoio, pulso END e retorno
  ao começo do motivo. A solicitação fica pendente até o relógio chegar.

PITCH de saída mantém a altura entre eventos e durante pausas, em 1 V/oct,
limitada a ±8. GATE dura 5–400 ms; uma amostra baixa no novo evento separa
articulações mesmo quando o relógio é mais rápido que essa duração. ACC é
um peso 0–1, maior no começo e distinto no final; numa pausa vale zero.
END dura 5 ms no último passo ou retorno solicitado. PHASE mantém a posição
normalizada de 0–1. Repetições mais rápidas que 5 ms podem fundir ENDs.

DIR −1 produz descida, zero usa o motivo semeado e +1 produz subida.
SPAN limita deslocamentos a 1–36 semitons de cada lado do apoio; DIR e VAR
compartilham esse limite. Primeiro e último passos não são omitidos por SPACE.
CAD=1 resolve sempre no apoio no último passo; CAD=0 conserva o percurso.
VAR=0, controles e estímulos fixos: as alturas do motivo se repetem.

O módulo não escolhe escala: para isso, PITCH → QUANTIZER.CV, com RANGE=1.
QUANTIZER.PITCH → voz. O gate da articulação continua vindo do PHRASE;
quantização de alturas iguais não deve apagar notas repetidas.

## Estado e escopo

prepare(), RST e mudança de SEED recomeçam a interpretação. O patch salva
controles e conexões, incluindo SEED; reabrir reconstrói o motivo. A posição
atual e a sequência de sorteios em andamento não são salvas, como nos outros
geradores sem estado de performance serializado. A identidade SEED fica fora
da VARIA, que pode interpretar os demais controles. Não há captura de áudio,
importação Scala, arquivo de escalas culturais ou inferência de emoção.

Processamento por evento, sem heap, I/O ou locks. Testes cobrem direção,
repetição, variação, cadência, pausas, estímulo, retorno, reset, blocos,
save/load e uma voz guiada pelos ataques do LISTEN. A qualidade de fraseado
precisa de escuta do autor; esses testes demonstram comportamento, não uma
certificação artística. Ornamentação, gramáticas de subida/descida específicas
de linguagem e um arquivo maior de alturas permanecem pendentes.
