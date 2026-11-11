# FRONTEIRA — onde a ideia de quem toca começa

**Família:** DECISION · **Módulo 79**
**Essência:** escuta uma melodia e marca o começo de cada célula (de cada
ideia), e o fim da frase quando vem o silêncio.
**Dossiê técnico:** [`../dossies/79_fronteira.md`](../dossies/79_fronteira.md)
· **Fonte:** `src/dsp/Fronteira.hpp`

---

## A ideia

Ouvimos uma fronteira onde algo **muda**: um salto depois de graus
conjuntos, uma nota longa depois de curtas, uma pausa. A FRONTEIRA mede
essas três mudanças nota a nota (o modelo de Cambouropoulos, a partir das
regras de agrupamento de Lerdahl & Jackendoff). Quando a força passa de
LIM, pulsa **FRONT**: começou uma célula nova. Para saber se uma nota é
fronteira ele precisa ouvir a seguinte, então o pulso chega **uma nota
depois**. Um silêncio longo (SIL) fecha a frase na hora, com um pulso em
**FIM**.

O PHRASE (#62) é outra coisa: ele *gera* um motivo. A FRONTEIRA *escuta*.

## Como cabear

```
SIGNAL-IN → SCOPE: PIT → FRONTEIRA (PTCH), ONS → FRONTEIRA (GATE)
FRONTEIRA (FRONT) → MIXER (FRASE)     modo GESTO: cada ideia nova muda o palco
FRONTEIRA (FIM)   → APOCRIFO (CIT)    o silêncio de quem toca abre uma citação
```

## Receitas

1. **O palco que segue a frase.** FRONT no FRASE do MIXER, modo GESTO.
2. **Acompanhamento que respira.** FIM no RST de um SEQUENCE: quando quem
   toca para, o acompanhamento recomeça junto com a próxima frase.
3. **Tamanho da ideia.** NOTAS (saída) no DEC de um ENVELOPE: frases longas
   pedem notas mais longas.
