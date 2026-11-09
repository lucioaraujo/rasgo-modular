# MARKOV — aprende e improvisa

**Família:** DECISION · **Módulo 66**
**Essência:** ouve uma melodia, conta de qual nota se vai para qual, e improvisa com esse vocabulário.
**Dossiê técnico:** [`../dossies/66_markov.md`](../dossies/66_markov.md)
· **Fonte:** `src/dsp/Markov.hpp`

---

## A ideia

Uma cadeia de Markov guarda **de onde se vai para onde**: depois do Dó veio
Mi três vezes e Sol uma vez. Para improvisar, a partir da nota atual ela sorteia
a próxima pelo que aprendeu. TEMP decide a ousadia: 0 repete o caminho mais
comum, 2 inventa. SURP diz quão inesperada foi cada nota. Sem nada ligado, o
MARKOV já toca: começa numa caminhada pentatônica.

Cada controle e cada jack estão explicados no LEARN do app (ligue LEARN e
aponte o mouse), nos quatro idiomas.

## Receitas

1. **A segunda voz.** `PHRASE.pitch` → `LRN.P`, `PHRASE.gate` → `LRN.G`;
   `MARKOV` → outro `OSC`. Uma voz que conversa com a primeira.
2. **Aprender de quem toca.** `SIGNAL-IN` → `SCOPE`; `SCOPE.PIT` → `LRN.P`,
   `SCOPE.CONF` → `LRN.G`: ele aprende da sua voz ou do seu instrumento.
3. **A surpresa move o patch.** `SURP` → `MIX` de um `SPACE`: as notas
   inesperadas ganham eco.
4. **Congelar o vocabulário.** Depois de ensinar, desligue LEARN e suba TEMP.
