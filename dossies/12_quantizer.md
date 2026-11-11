# Dossiê — Módulo 12: Quantizador de escala (`QUANTIZER`)

**Família:** DECISION / PERCEPTION
**Estado:** **implementado — marco 2** (2026-09-02)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Quantizer.hpp`, `tests/test_quantizer.cpp`

## Estado da implementação (marco 2)

Feito: transforma CV contínua em **alturas de uma escala musical**. A
saída `pitch` é em **oitavas** (1 V/oct) — alimenta o `rate_mod` de uma
voz e a transposição fica certa.

- **entrada:** `cv ∈ [−1,1] → semitons = cv·range·12`; `transpose` (1
  V/oct) soma oitavas;
- **snap:** acha o pitch da escala (em qualquer oitava, ±1 da base) mais
  próximo dos semitons pedidos; `root` desloca a tônica;
- **histerese:** só troca de nota se afastou mais que
  `hysteresis·½·(passo médio da escala)` do valor preso — não tremula
  entre graus vizinhos;
- **sample-and-hold:** se `trigger` está conectado, a nota só é
  reavaliada na borda de subida; senão é contínua;
- **glide:** `glide` → portamento exponencial até a nova nota (a cola de
  theremin do `RASGO_SYNTH`);
- **`gate`:** pulso de ~5 ms quando a nota muda (dispara envelopes);
  **`semitone`:** o valor em `[−1,1]` aproximado, pra outros usos.

**12 escalas curadas** — subconjunto das ~39 tabelas pesquisadas em
`RASGO_SYNTH/rasgo-synth-core/src/sequencer/Scales.hpp` (graus cruzados
com nomenclatura de teoria/jazz, com 3 erros de uma fonte anterior
corrigidos lá): Cromática, Maior, Eólio, Dórico, Frígio Dominante,
Lídio, Menor Melódica, Pentatônica menor, Pentatônica maior, Hirajoshi,
Tons Inteiros, Oitava. Leve de propósito, não a máquina inteira
([[feedback_generative_design_light_touch]]).

Determinístico (sem RNG).

**Testes (7/7 alvos, Debug + Release):** com escala Maior, uma rampa de
CV de −1 a 1 → toda saída é semitom **inteiro** e pertence ao conjunto
`{0,2,4,5,7,9,11}` mod 12, e é **monotônica não-decrescente**; `root = 3`
→ as notas são `(grau maior + 3) mod 12`; com `trigger` conectado e CV
variando sempre, o `pitch` **não muda entre pulsos** (0 mudanças) e
muda em ≥ 3 pulsos; `glide = 0,5` limita o salto de altura por amostra a
< 0,02; dois renders byte-idênticos; integração no grafo
(`TURING.cv → QUANTIZER → voz.rate_mod` = melodia pentatônica); painel
fecha (12 HP), 12 escalas.

**Pendências (candidatos, não controles fictícios):** movimento
harmônico (as 6 técnicas de `HarmonicWanderer.hpp` — Coltrane, tritone
sub, mediante cromática/Jobim, intercâmbio modal — a escala/tônica
mudando ao longo da peça); escala definida por máscara de 12 bits (o
usuário liga/desliga notas); "strum"/acorde (várias saídas de pitch
simultâneas de um voicing — `MelodyVoicingBank.hpp`); quantização com
peso por grau (probabilidade de cair em cada nota, tipo Marbles `t`);
saída em Hz além de oitavas; entrada de `scale` como CV.

---

## 1. Problema musical e papel no fluxo

`TURING` e `DECISION` produzem CV em degraus — mas degraus iguais em
tensão não são degraus iguais em música. Sem um quantizador, a "melodia"
generativa é atonal por acidente. O `QUANTIZER` é o que faz o acaso
soar **numa tonalidade** — e com `root`/`scale` modulados, é o que faz a
peça ter harmonia que se move. É o elo entre "gerar variedade" e
"gerar música original" (a finalidade declarada da v1).

Papel: fica entre a fonte de CV (`TURING`/`DECISION`/LFO) e a fonte de
altura (`FUNCTION.rate_mod`, `STRING.freq_mod`, `MATTER.freq_mod`);
`trigger` do `CLOCK` alinha as notas ao ritmo; `gate` de mudança de nota
dispara o `ENVELOPE`.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **`RASGO_SYNTH/rasgo-synth-core/src/sequencer/Scales.hpp`** (projeto irmão RASGO) | as tabelas de graus — 39+ escalas reais, cada grau cruzado com nomenclatura de teoria/jazz, com 3 erros de uma fonte anterior corrigidos e documentados. Aqui um subconjunto curado (as tabelas são portáveis "em espírito"; o código não é incluído entre projetos) | projeto próprio (AGPLv3 / GPLv3); só as tabelas de intervalos, que são fato musical de domínio público |
| **Quantizadores de hardware** (Doepfer A-156, Intellijel Scales, Mutable Marbles `t`) | histerese/deadband pra não tremular; sample-and-hold por trigger; `root` e transposição | hardware, estudo de comportamento |
| **`RASGO_SYNTH/dsp/ThereminVoice.hpp`** | glide exponencial até a altura-alvo (coeficiente derivado do sample rate real) — nunca "salta", sempre desliza | projeto próprio; a técnica (glide de 1º ordem) é padrão |

## 3. Modelo — matemática, estados, extremos

Por amostra:
```
wantSemi = cv·range·12 + transpose·12
update   = trigger conectado ? (borda de subida) : sempre
se update:
    snapped = snap(wantSemi, escala, root)
    se !init  ou  |snapped − held| > hysteresis·½·(12/comprimento):
        se mudou: gateCountdown = 5 ms;  held = snapped;  init = true
glide:  se glide=0  → glideSemi = held
        senão       → glideSemi += (held − glideSemi)·glideCoeff
pitch    = glideSemi / 12          (oitavas)
gate     = gateCountdown>0 ? 1 : 0  (decrementa)
semitone = glideSemi / 24
```
`snap(semi)`: varre os graus da escala nas oitavas `⌊semi/12⌋ ± 1`,
`cand = oct·12 + grau + root`, devolve o `cand` mais próximo de `semi`.

`glideCoeff = 1 − exp(−1/(glide·0,2·sr))` (τ até 200 ms).

**Estados:** `heldSemi`, `glideSemi`, `prevTrigger`, `gateCountdown`,
`initialized`. Sem alocação, sem RNG.

**Extremos.** `range = 6` → `cv = 1` pede +72 semitons (6 oitavas), o
`snap` acha o grau certo. `scale = 0` (cromática) → todo semitom é
válido → `hysteresis` importa muito (sem ela, tremula a cada micro-
mudança de CV). `scale = 11` (Oitava) → só a tônica em cada oitava
(arpejo de oitavas). `glide` alto + notas rápidas → o `pitch` fica atrás
do `held` (correto — portamento longo). `hysteresis = 1` → precisa
afastar meio passo pra trocar (nota "gruda"). Reset → tudo zero,
`initialized = false` (a 1ª avaliação sempre passa).

## 4. Três modos obrigatórios

- **Autônoma:** sem `trigger`, quantiza continuamente — um LFO triangular
  na entrada já vira um arpejo da escala. `glide` alto = uma linha que
  desliza entre as notas (theremin).
- **Performance:** `scale`, `root`, `range` são os macros — trocar de
  escala ao vivo recolore a peça inteira; `root` transpõe; `range`
  aperta ou abre o âmbito. `glide` é um gesto expressivo (staccato ↔
  legato deslizante).
- **Híbrida:** `trigger` do `CLOCK` trava as notas no ritmo; `transpose`
  de um `DECISION` lento move a tonalidade por seção; `gate` de mudança
  de nota → `ENVELOPE` (cada nota nova é articulada).

## 5. Portas, parâmetros, limites

**Entradas:** `cv` (Audio, a quantizar), `transpose` (Control, 1 V/oct),
`trigger` (Control — S&H; se ausente, contínuo).
**Saídas:** `pitch` (Audio, oitavas / 1 V/oct), `gate` (Control, pulso
na mudança de nota), `semitone` (Audio, ~[−1,1]).
**Parâmetros:**
| id | faixa | default | o que faz |
|---|---|---|---|
| `scale` | 0–11 | 1 (Maior) | índice da escala curada |
| `root` | 0–11 st | 0 | tônica |
| `range` | 1–6 oct | 2 | oitavas que a CV [−1,1] cobre |
| `glide` | 0–1 | 0 | portamento até a nova nota |
| `hysteresis` | 0–1 | 0,3 | deadband contra tremulação |

**Limites:** `pitch` limitado por `range` (≤ ±6 oitavas). CPU: `snap` é
O(graus·3) por amostra quando `update` — barato (≤ 36 comparações); com
`trigger` conectado, só nos pulsos. Sem alocação.

## 6. Alternativas descartadas

- **As 39 escalas + os modos de `HarmonicWanderer`/`MelodyVoicingBank`
  já no marco 2:** é a "máquina inteira" que o autor prefere não
  reusar por atacado. 12 escalas curadas cobrem o essencial; movimento
  harmônico é 2ª camada.
- **Escala como enum fixo em vez de índice float:** o parâmetro precisa
  ser modulável (CV → `scale`), então `float` + `lround` + clamp.
- **Quantizar em Hz:** oitavas (1 V/oct) casam direto com o `rate_mod`
  exponencial das vozes; Hz exigiria uma conversão em cada consumidor.
- **Sem histerese:** com escalas de passo pequeno (cromática, tons
  inteiros) a saída tremula audivelmente com qualquer ruído na CV.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** com qualquer escala, `pitch·12` é sempre inteiro e
pertence ao conjunto de graus (mod 12, com `root`); a saída é monotônica
em relação à CV crescente; com `trigger`, zero mudanças de `pitch` entre
pulsos; `glide` limita a derivada de `pitch`; `gate` pulsa só na
mudança; dois renders byte-idênticos; sem alocação.

**Escuta:** a melodia quantizada soa "numa tonalidade" ou ainda
aleatória? trocar de escala ao vivo recolore de forma musical? `glide`
soa como um instrumentista escorregando entre notas ou como um bug de
pitch? a histerese elimina a tremulação sem "engolir" mudanças
reais? `TURING → QUANTIZER → STRING` já é uma frase?

## 8. Integração e painel

Classe `Quantizer` (`type()` = `"QUANTIZER"`), 3 entradas, 3 saídas, 5
parâmetros, 12 escalas (`Quantizer::scales()`). `panel()` próprio
(12 HP: display da escala + SCALE/ROOT/RANGE, GLIDE/HYST, jacks).
Testado isolado (snap, root, S&H, glide, determinismo) antes do patch.
É o módulo que fecha o caminho "acaso → música": `TURING`/`DECISION` →
`QUANTIZER` → `STRING`/`MATTER`/`FUNCTION`, com `CLOCK` no `trigger`.

## 9. Proposta do lote da harmonia: crivos e afinações (11/11/2026, implementado; decisões abaixo)

Da auditoria (`AUDITORIA_modulos_e_cardinal.md`: "peneiras de Xenakis,
afinações (justa, pitagórica, EDO)", aprovado) e do estudo da família §1
(SIEVE: Batruc e Studio têm o conceito).

1. **CRIVO** (a escala nº 12, depois de "Octave").
   - As alturas permitidas são uma **união de duas classes de resíduo**
     sobre a linha dos semitons: n ≡ R1 (mod M1) **ou** n ≡ R2 (mod M2), com
     M1 e M2 de 2 a 13. É a peneira de I. Xenakis (*Formalized Music*,
     1963–71).
   - Quando M não divide 12, a escala **não se repete na oitava**: com 3 ∪
     5, o desenho só volta depois de 15 semitons. É uma escala que o
     QUANTIZER hoje não faz.
   - Os módulos e os resíduos ficam **visíveis** como knobs (M1, R1, M2, R2):
     o crivo do Batruc diz que um "preset Xenakis" sem módulos e resíduos
     não explica o processo.
   - O visor mostra os pontos do crivo.
   - Pergunta: união só, ou também uma chave de interseção (∩)?
2. **AFIN** (afinação, 6 posições). A nota escolhida pela escala é
   reafinada:
   - **12-EDO** (o de hoje, o padrão);
   - **Justa** (razões de 5-limite: 16/15, 9/8, 6/5, 5/4…);
   - **Pitagórica** (quintas 3/2 puras);
   - **Mesotônica** de ¼ de coma.

   Essas três medem **a partir da tônica (ROOT)**. Com o HARMONY ou o TOM
   mudando a tônica, a afinação justa **acompanha a modulação**: uma
   afinação adaptativa, que um teclado fixo não faz. E há ainda:
   - **19-EDO** e **31-EDO**, que trocam a própria grade. A escala escolhida
     é mapeada para os passos mais próximos da nova divisão (a terça de
     31-EDO é quase justa).

   A saída PTCH já é em V/oct, então a reafinação chega a qualquer voz.
   - Pergunta: arquivos Scala (.scl) ficam de fora por ora? (pedem leitura de
     arquivo e um seletor). Recomendação: fora, por enquanto.

**Painel:** passa de 10 para 14 HP (AFIN e os quatro knobs do crivo numa
fileira nova). Patches antigos abrem iguais: tudo novo vem em 12-EDO e o
crivo só entra com SCALE = 12.

**Decisões e implementação (11/11/2026).** O autor, com "avance", seguiu as
recomendações: crivo com **união e interseção** (chave AND) e arquivos
**Scala fora** por ora.
- **Crivo:** a escala nº 12 (`kSieveIndex`), só pelo knob; a CV de escala
  continua 0–11. A busca vai à nota do crivo mais próxima (empate: a de
  baixo), em até ±40 semitons. Uma interseção vazia vira união.
- **Afinação:** aplicada à nota segurada, antes do glide.
  - Tabelas em cents relativas ao 12-EDO, por grau a partir da tônica:
    Justa (16/15, 9/8, 6/5, 5/4, 4/3, 45/32, 3/2, 8/5, 5/3, 9/5, 15/8);
    Pitagórica (cadeia de quintas, trítono 729/512); Mesotônica ¼ de coma
    com bemóis na 2ª, 3ª, 6ª e 7ª menores (a 1ª versão tinha a 6ª menor como
    sol♯, 27 cents baixa nas escalas menores; corrigido antes do commit).
  - 19-EDO e 31-EDO são ancorados na **tônica**.
- **Painel:** 14 HP; AFIN na fileira do meio; M1, R1, M2, R2 numa coluna à
  direita; chave AND.
- **Seed:** afinação e crivo fora do sorteio, da mutação e do VARIA; as
  seeds soam iguais.
- **Teste** (`test_quantizer`, `testSieve`, `testTunings`):
  - 3 ∪ 5 só dá múltiplos de 3 ou 5 e não se repete na oitava;
  - 2 ∩ 3 dá múltiplos de 6;
  - a interseção vazia volta à união;
  - terça justa 386,31 cents, pitagórica 407,82;
  - a afinação justa acompanha ROOT = Ré;
  - 19-EDO cai na grade.
