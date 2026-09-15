# MATRIX — matriz de roteamento 4×4

**Família:** ROUTE · **Módulo 33**
**Essência:** o oposto do cabo — **todas** as conexões possíveis entre 4
fontes e 4 destinos, cada uma com um botão. Modulação em bloco, sem
espaguete.
**Dossiê técnico:** [`../dossies/33_matrix.md`](../dossies/33_matrix.md)
· **Fonte:** `src/dsp/Matrix.hpp`

---

## A ideia

Em vez de uma conexão de cada vez, o `MATRIX` te dá a grade inteira: cada
cruzamento fonte×destino é um ganho (atenuversor). É como se pensa
modulação em bloco — "essa fonte um pouco em todo lugar", "troca a fonte
de tudo de uma vez". Com `DRIFT`, vira uma rede de modulação que evolui
sozinha. No painel é uma **grade clicável**.

## Por dentro

**A fórmula, devagar:** `out_k = LEVEL · sat(Σ_j in_j · g_jk)` diz que
**cada saída** é a soma das **4 entradas**, cada uma multiplicada pelo
seu próprio ganho de célula (`g_jk`, o cruzamento entrada-j×saída-k) —
literalmente, cada saída "escuta um pouco de cada entrada", na
proporção que a grade decidir. As 16 células (−1..1) começam na
**identidade**: só a diagonal (`g11`, `g22`, `g33`, `g44`) em 1 e o
resto em 0 — o que corresponde exatamente a `in1 → out1`, `in2 → out2`
etc., ou seja, o `MATRIX` "desligado" se comporta como 4 cabos diretos
comuns. Abrir qualquer outra célula é literalmente **acrescentar** uma
conexão a mais que não existiria com cabos comuns (uma entrada indo
pra **mais de uma** saída, ou várias entradas somando na **mesma**
saída) — daí "todas as conexões possíveis de uma vez, cada uma com um
botão". Um ganho **negativo** na célula inverte a fase daquela
contribuição específica antes de somar.

`NORM` resolve o mesmo problema do `1/√N` do `CHORD` (#26): somar
várias entradas ativas na mesma saída tende a deixar o nível mais alto
quanto mais células você abrir — `NORM` compensa isso automaticamente,
mantendo o nível de cada coluna razoavelmente constante não importa
quantas células estejam ativas ali.

**`RING`, o que muda estruturalmente:** em vez de **somar** as
contribuições de uma coluna, ele faz a coluna calcular o **produto**
das entradas ativas — a mesma multiplicação de sinais do `RingMod` de
cabo (`RELACAO_DE_CABO.md` §1.1), só que aqui generalizada pra até 4
fontes numa única saída de uma vez (um "ring-mod de 4 quadrantes"), em
vez de só duas.

`SAT` (a mesma curva "achata suavemente" de sempre) garante que, se
você cabear uma saída de volta numa entrada (um feedback matricial —
`OUT1 → IN1`, marcado pelo painel como qualquer outro feedback de
cabo), o laço converge pra um ciclo-limite em vez de crescer sem
controle. `DRIFT` aplica o mesmo passeio lento correlacionado de
sempre aos 16 ganhos simultaneamente — a rede de modulação inteira
"respira" e se reconfigura sozinha ao longo do tempo.

## Os jacks, um a um

### Entradas

- **`IN1`**–**`IN4`** (áudio) — as 4 fontes. **Plugue aqui:** vozes,
  LFOs, envelopes — ou uma mistura (áudio numa linha, CV noutra).

### Saídas

- **`OUT1`**–**`OUT4`** (áudio) — os 4 destinos, cada um `LEVEL · sat` da
  soma da sua coluna. **Plugue em:** canais do `MIXER` (roteamento de
  áudio), ou `*_mod` de vários módulos (um painel de modulação).

## Os controles

**A grade 4×4** (clicável no painel) — cada célula é o ganho de
`IN_j → OUT_k`. Arraste vertical numa célula ajusta o ganho (−1..1).

**LEVEL** (0–2) — o ganho geral de todas as saídas.

**NORM** (0–1) — normaliza o nível por coluna (a soma não infla quando
você abre várias células).

**RING** (0–1) — a coluna vira **produto** em vez de soma — ring-mod de
4 quadrantes entre as fontes ativas.

**SAT** (0–1) — saturação (segura a matriz quando você a realimenta).

**DRIFT** (0–1) — passeio lento dos 16 ganhos.

## Como cabear

**Painel de modulação:**
```
LFO      → MATRIX (IN1)
ENVELOPE → MATRIX (IN2)
NOISE.smooth → MATRIX (IN3)
DRIFT.a  → MATRIX (IN4)

MATRIX (OUT1) → FILTER (FC)
MATRIX (OUT2) → VCA (cv)
MATRIX (OUT3) → SHAPE (FCV)
MATRIX (OUT4) → SPACE (mix_mod)
```
Cada knob da grade decide quanto de cada fonte vai a cada destino.

**Ring-mod de 4 osciladores:**
```
4 × OSC → MATRIX (IN1..4)     RING alto
MATRIX (OUT1) → MIXER
```

## Potencializar

- **Troca de fonte de tudo:** feche a linha de uma fonte e abra a de
  outra — todos os destinos trocam de modulador de uma vez.
- **Rede que evolui:** `DRIFT` ~0,3 — as relações de modulação passeiam
  ao longo de minutos; o patch "se reconfigura".
- **Feedback matricial:** `OUT1 → IN1` (o painel marca como feedback) +
  `SAT` alto — a matriz ressoa; ajuste as células de ouvido.
- **Coluna como ring:** `RING` só numa coluna (via automação parcial não
  há — mas com `RING` no talo, uma saída fica ring-mod e as outras
  seguem o `sat`).

## Se você conhece o Eurorack

Faz o papel da matriz de pinos do EMS Synthi / ARP 2500, ou de um matrix
mixer (Doepfer A-138m, Befaco, Erica). O `norm` por coluna vem do
Serge/Buchla. A camada matriz também existe no motor do Rasgo desde o
marco 2. Distinto do `SWITCH` (uma fonte por vez) e do `MULT` (uma
fonte, 4 cópias).
