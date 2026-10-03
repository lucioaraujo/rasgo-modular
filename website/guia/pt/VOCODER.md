## O que é
O VOCODER faz um som falar com a voz de outro. Ele escuta um som, normalmente uma voz, e mede quanta energia há em cada faixa de frequência; depois aplica esse desenho a um segundo som, rico em harmônicos. O resultado é o som do segundo com a fala do primeiro: o efeito de voz robótica de tanta música eletrônica.

## Como pensar nele
Ele precisa de duas entradas: CAR, a portadora, que é o som que vai falar (uma serra, um acorde), e MOD, que é quem fala (uma voz pelo SIGNAL-IN, um tambor, uma gravação). Com poucas faixas, o resultado é grosso e robótico; com muitas, a fala fica clara. Nada na entrada CAR? Ele usa uma serra interna, afinável pela entrada PIT.

## Controles
- BANDS: quantas faixas de frequência, de 4 a 20. Poucas soam robóticas; muitas deixam a fala inteligível.
- SHIFT: muda o tamanho da voz resultante, de grande a pequena, sem mudar o que é dito.
- ATK: a rapidez com que cada faixa responde. Curto, as consoantes saem secas; longo, tudo amacia.
- REL: quanto tempo cada faixa leva para soltar. Curto é nítido; longo borra as sílabas num pad.
- SIBIL: quanto do agudo da voz passa direto, para os S e T ficarem claros.
- FRZ: congela o desenho atual. A portadora continua dizendo a última sílaba indefinidamente.
- MIX: a mistura entre a portadora original e a portadora falando.
Entradas: CAR, a portadora; MOD, a voz que fala; PIT, a altura da serra interna quando não há nada em CAR.

## Experimente
1. Ligue a saída SAW de um OSC em CAR, e a saída L do SIGNAL-IN, com um microfone, em MOD. Ligue OUT numa entrada do MIXER e fale.
2. Baixe BANDS para 6 e ouça a voz ficar robótica; suba para 20 e ela fica clara.
3. Suba REL: as palavras se emendam num som contínuo.
4. Sem microfone, ligue a saída OUT de um DRUM em MOD. Cada batida abre a portadora no seu próprio timbre.
