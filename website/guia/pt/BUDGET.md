## O que é
O BUDGET dá às vozes um orçamento de eventos. Quatro gates entram e saem, mas por ciclo só passam TETO eventos no total. Quando o orçamento acaba, os gates seguintes ficam de fora até o ciclo recomeçar.

## Como pensar nele
Densidade não é qualidade. O BUDGET faz o patch escolher: com pouco orçamento, cada nota vale mais. Há duas economias. Compete: quem chega primeiro leva. Coopera: cada voz que toca tem a sua parte, e só passa da parte se as outras ainda não gastaram a delas.

## Controles
- TETO: quantos eventos cabem em cada ciclo, de 1 a 32.
- COOP: desligado, as vozes competem; ligado, cooperam.
- CICLO: a duração do ciclo, de 0,25 a 16 s, quando nada está ligado em CLK.
Entradas: G1 a G4 recebem os gates das vozes, CLK recomeça o ciclo a cada pulso. Saídas: O1 a O4 são os gates que passaram, SALDO o que resta do orçamento, FIM um pulso quando ele acaba.

## Experimente
1. Ligue os gates de duas vozes em G1 e G2, e O1 e O2 nos envelopes delas.
2. Com TETO em 6, ouça as vozes disputarem o ciclo.
3. Ligue COOP: as duas passam a dividir o orçamento.
4. Ligue SALDO no corte de um FILTER: o som escurece à medida que o orçamento se gasta.
