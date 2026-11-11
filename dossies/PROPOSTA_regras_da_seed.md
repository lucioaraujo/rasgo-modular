# Proposta — REGRAS da seed: garantia de limite e gene Heisenberg

Data: 11/11/2026. Estado: **aprovada** ("ok para as 3 perguntas": os cinco papéis e as chances; 19/31-EDO no Heisenberg; a garantia vale para toda seed, cada papel só onde cabe). Implementação no lote da seed. Origem: estudo da
família §4.8.3, item 5 (aprovado como direção: "gosto de todos"). Prior art:
`PieceRenderer.hpp` do Rasgo Synth Studio, "Garantia de limite" e "Fase 71 —
Heisenberg" (mesmo autor; conceito, não código). As duas mexem em **como
toda seed se compõe**, então só entram com o ok do autor.

## 1. Garantia de limite

**No Studio:** "a peça precisa, de algum jeito seu, se arriscar de verdade".
Quatro mecanismos de risco são sorteados de forma independente. Se nenhum
saiu, um é forçado, escolhido pela seed: "a rede de segurança decide QUE
algo aconteça, nunca QUAL".

**No Modular, hoje:** os módulos de ruptura existem no rack (COHERENCE,
APÓCRIFO, BLEFE, a FIBRA do ADDITIVE, o HARMONY que modula), mas a seed
**não os cabeia**. Uma seed pode sair mansa.

**Proposta:** cinco **papéis de limite**, cada um sorteado com uma chance
baixa e independente, sobre o patch que a seed já montou:

| Papel | O que a seed cabeia | Chance |
|---|---|---|
| **Citação** | MIXER → APOCRIFO → MASTER, CHANCE 0,15–0,4, modo sorteado, IDADE 20–40 s | 0,15 |
| **Ruptura da ordem** | a linha da voz (QUANTIZER PTCH/GATE) → COHERENCE; COL → RST do sequenciador da voz | 0,15 |
| **Modulação** | HARMONY sai do jazz modal (quase parado) para outro movimento (inclusive o NEO-RIEMANN) | 0,15 |
| **Confiança** | o gate da voz → BLEFE; RESP → o nível da contravoz (ela responde enquanto confia) | 0,10 (só com contravoz) |
| **Rasgo da matéria** | quando há ADDITIVE: um FUNCTION lento → TENS (o timbre se esvazia ao longo da peça); RST no fim da frase | 0,10 (só com ADDITIVE) |

Se **nenhum** sair, a seed força **um**, escolhido por ela, entre os que
cabem no patch. Sai no seeds.log (`limite: CITAÇÃO`, por exemplo).

**Crivo de qualidade:** cada papel é medido na régua de qualidade da seed
(`test_seed_quality`): sem clipe, sem silêncio, sem clique. A garantia não
pode piorar o som.

## 2. Gene Heisenberg

**No Studio:** em ~25 % das peças, um único **equilíbrio** b decide ao mesmo
tempo a precisão do ritmo e a da afinação, **inversamente**. Perto de 0, a
grade é rígida e a afinação vaga. Perto de 1, a afinação é presa e o ritmo
fraturado. A peça nunca tem as duas precisões ao mesmo tempo: é uma troca de
incerteza, não dois sorteios independentes.

**Proposta para o Modular** (uma corrente de sorteio própria, `stream 17`,
para não mudar as outras escolhas da seed):
- **Quando entra:** em 25 % das seeds.
- **Afinação vaga** (b perto de 0):
  - DRIFT da voz alto (até 0,6);
  - GLIDE do QUANTIZER curto;
  - em parte das seeds, a AFIN em **19-EDO** ou **31-EDO**: a afinação vaga
    como outra grade, não como erro.
- **Ritmo fraturado** (b perto de 1):
  - DRIFT e SWING do CLOCK altos;
  - a afinação presa: DRIFT 0, AFIN **Justa** (a mais "presa" de todas).
- **Escala:** entre os dois extremos, uma escala contínua.
- **Registro:** sai no seeds.log (`heisenberg 0,73`).

## 3. O que não muda

As seeds **anteriores mudam**: é inevitável, porque são regras novas sobre
toda seed. A identidade da seed (`test_seed_identity`: a mesma seed dá o
mesmo patch) continua garantida.

## Perguntas ao autor

1. Os cinco papéis e as chances estão bons, ou algum fica de fora?
2. O Heisenberg pode usar 19/31-EDO como "afinação vaga", ou só o DRIFT?
3. A garantia vale para **toda** seed, ou só para as de 2 vozes ou mais (a
   de 1 voz é mais íntima, e a ruptura poderia sobrar)?

## Implementação (11/11/2026)

`apps/panel/PatchSeed.hpp`:
- **Correntes próprias.** Os papéis usam as correntes 30–53 e o Heisenberg
  as 17–19. A escolha da escada e do acento usa as 20–28. Nada que vem
  antes no patch muda por causa delas.
- **Quando cada papel cabe:**
  - CITAÇÃO: há MIXER, MASTER e APÓCRIFO.
  - ORDEM: o sequenciador que alimenta o QUANTIZER tem RST livre.
  - MODULAÇÃO: o HARMONY ainda não está cabeado.
  - CONFIANÇA: há contravoz.
  - RASGO: o ADDITIVE é voz ou contravoz.
- **Como cada papel se cabeia:**
  - CITAÇÃO: o APÓCRIFO entra entre o MIXER e o MASTER; se a ligação
    falhar, o MIXER volta direto ao MASTER, e a seed nunca fica sem som.
  - CONFIANÇA: o BLEFE entra entre o gate do gerador da contravoz e o
    gatilho dela. A janela é de 0,05–0,15 s, e é a resposta que chega um
    pouco depois, dita.
  - MODULAÇÃO: o HARMONY mantém a escala da seed (S-LO = S-HI = a escala
    sorteada) e move a tônica.
- **Registro:** o seeds.log ganha `limite …`, `heisenberg …` e, quando
  há, `LADDER`/`ACCENT`.
