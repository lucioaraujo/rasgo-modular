# KEY (TOM) — o tom de quem toca

**Família:** DECISION · **Módulo 73**
**Essência:** escuta uma linha de notas e diz em que tom ela está (tônica,
maior ou menor), no formato que o QUANTIZER entende.
**Dossiê técnico:** [`../dossies/73_key.md`](../dossies/73_key.md)
· **Fonte:** `src/dsp/Key.hpp`

---

## A ideia

O patch toca na escala que a seed escolheu, e quem canta ou toca por cima
pode estar noutra. O TOM fecha esse ciclo: ele ouve quem toca, estima o tom
e entrega RAIZ e ESCALA ao QUANTIZER. A partir daí, o patch passa a
responder no tom de quem toca.

Por dentro, ele conta quanto tempo cada uma das 12 notas soou (a nota longa
pesa mais que a de passagem, como no Maravi) e compara esse retrato com os
perfis de tonalidade de Krumhansl e Kessler (1982). O tom só muda quando
outro vence por ESTAB notas seguidas, para não tremular entre tons
vizinhos. MEM é quanto tempo ele lembra: curto segue as modulações, longo
fica firme.

A CERTEZA diz quando o tom é ambíguo (tons relativos, cromatismo), em vez
de esconder isso.

O visor mostra as 12 notas, a tônica em destaque, a barra de certeza e o
nome do tom ("Dm" = Ré menor).

Cada controle e cada jack estão explicados no LEARN do app (ligue LEARN e
aponte o mouse), nos quatro idiomas.

## Como cabear

```
SIGNAL-IN (L)   → SCOPE (IN)
SCOPE (PIT)     → KEY (PTCH)
SCOPE (ONS)     → KEY (GATE)
KEY (ROOT)      → QUANTIZER (ROOT)
KEY (SCL)       → QUANTIZER (SCL)
```

Ligue o ON do SIGNAL-IN. Em poucas notas, o patch vai para o tom de quem
canta.

## Receitas

1. **O patch que acompanha.** O cabeamento acima. Cante uma melodia em Ré
   menor: o visor mostra "Dm" e o QUANTIZER passa a tocar em Ré menor.
2. **A forma pela modulação.** MUD → FRASE do `MIXER` (modo GESTO): cada
   troca de tom muda o palco.
3. **Afirmar quando está claro.** CERT → o nível de um `VCA` numa voz de
   apoio: quanto mais claro o tom, mais o acompanhamento aparece.
4. **Sem microfone.** Um `MARKOV` ou `TURING` → `QUANTIZER` → KEY → um
   segundo `QUANTIZER`: a segunda voz segue o tom que a primeira "parece"
   ter.

## Se você conhece o Eurorack

Não há módulo de detecção de tonalidade no Cardinal. Fora do Eurorack, é o
algoritmo Krumhansl–Schmuckler, usado em análise musical. Aqui ele é causal,
ao vivo, e o erro vira material: um tom mal inferido é uma harmonia que
responde torto.
