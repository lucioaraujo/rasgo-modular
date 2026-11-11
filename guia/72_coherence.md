# COHERENCE — a ordem que se rompe

**Família:** DECISION · **Módulo 72**
**Essência:** escuta uma linha de notas e, quando ela fica previsível
demais por tempo demais, rompe: um pulso em COLAPSO.
**Dossiê técnico:** [`../dossies/72_coherence.md`](../dossies/72_coherence.md)
· **Fonte:** `src/dsp/Coherence.hpp`

---

## A ideia

Costuma-se pensar que a ruptura nasce do caos. O COHERENCE faz o contrário:
a **ordem extrema** também rompe. Ele guarda as últimas notas (a janela,
JAN) e mede quanto elas são previsíveis. Uma nota repetida dá ORDEM 1, e as
12 notas por igual dão 0. A medida é a entropia de Shannon, a mesma
grandeza da teoria da informação.

Enquanto a ORDEM passa do LIMIAR (LIM), a PRESSÃO sobe, um pouco a cada
nota. Depois de PACIÊNCIA (PAC) notas seguidas, ela rompe: sai um pulso em
COL. A ruptura deixa **cicatriz**: a janela esquece a metade mais antiga e,
por CIC notas, a pressão não volta a subir.

O módulo não soa sozinho. Ele decide **quando**, e o cabo decide **o
quê**.

Cada controle e cada jack estão explicados no LEARN do app (ligue LEARN e
aponte o mouse), nos quatro idiomas.

## Como cabear

```
QUANTIZER (PTCH) → COHERENCE (PTCH)
QUANTIZER (GATE) → COHERENCE (GATE)
COHERENCE (COL)  → SEQUENCE (RST)    a linha que se repete demais se refaz
COHERENCE (PRES) → FILTER (FC)       a tensão se ouve crescer
```

Sem o cabo de GATE, conta uma nota cada vez que a altura muda.

## Receitas

1. **O ostinato que se cansa.** Um `SEQUENCE` curto → `QUANTIZER` →
   `COHERENCE`; COL → RST de um `TURING` que alimenta outra voz. A repetição
   acumula até que a outra voz se refaz.
2. **Tensão audível.** PRES → `cutoff_mod` de um `FILTER` ou `LADDER`: o
   timbre abre enquanto a linha se repete e fecha na ruptura.
3. **Surpresa como acento.** SURP → o nível de um `VCA`: a nota rara soa
   mais forte.
4. **Com quem toca.** `SIGNAL-IN` → `SCOPE` (PIT, ONS) → COHERENCE: a
   repetição de quem toca dispara o `GESTO` do PALCO no `MIXER` (COL →
   FRASE).

## Se você conhece o Eurorack

Não há equivalente direto. O mais próximo são os comparadores e os
detectores de padrão, mas nenhum mede a previsibilidade de uma linha. A
ideia vem do `CoherenceCollapse` do Rasgo Synth Studio e da expectativa
musical de D. Huron.
