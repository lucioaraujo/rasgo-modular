# APOCRIFO (APÓCRIFO) — o patch cita o próprio passado

**Família:** SPACE · **Módulo 77**
**Essência:** guarda até 60 s do que passa e, de vez em quando, devolve um
trecho de longe que rasga o presente.
**Dossiê técnico:** [`../dossies/77_apocrifo.md`](../dossies/77_apocrifo.md)
· **Fonte:** `src/dsp/Apocrifo.hpp`

---

## A ideia

O LOOPER repete o agora. O APÓCRIFO traz o **passado distante**, de pelo
menos IDADE segundos atrás, como uma ruptura. Tem três modos:

- **CORROÍDA** — o trecho volta **ao contrário** e roído, e o presente se
  abaixa por baixo dele: uma memória corrompida.
- **CORTE** — um corte de montagem: o trecho volta **limpo**, uma vez, e as
  duas emendas caem no cruzamento de zero, como a lâmina do editor de fita.
- **PARÊNTESE** — o **agora** gagueja por um instante enquanto o patch
  segue por baixo, e volta exatamente onde o patch já está.

A citação vem por um gatilho em CIT ou sozinha, pela CHANCE (até uma a cada
~10 s). A saída IDADE diz de quão longe ela veio. A memória guarda a
entrada, nunca a própria saída: o apócrifo não cita a si mesmo.

Cada controle e cada jack estão explicados no LEARN do app, nos quatro
idiomas.

## Como cabear

```
MIXER (L+R) → APOCRIFO (IN)
APOCRIFO (OUT) → MASTER (IN)
```

## Receitas

1. **A abertura que volta.** CHANCE 0,3, MODO CORROÍDA, IDADE 30: de vez em
   quando, a peça de meio minuto atrás rasga o meio.
2. **Montagem.** MODO CORTE, o COL do `COHERENCE` em CIT: quando a linha
   fica previsível demais, a peça corta para o passado.
3. **Parêntese no compasso.** MODO PARÊNTESE, DUR 0,6, o EOS de um
   `SEQUENCE` em CIT: o fim de cada frase gagueja.
4. **Saber de onde veio.** IDADE (saída) no FC de um FILTER: quanto mais
   antiga a citação, mais escura.
