# HALL — reverberação FDN

**Família:** SPACE · **Módulo 46**
**Essência:** a densidade lisa de uma rede de atraso realimentada bem
afinada — o hall de concerto, a placa, o *ambience* que cola uma
mixagem, a nuvem infinita de pad congelado. Estéreo.
**Dossiê técnico:** [`../dossies/46_hall.md`](../dossies/46_hall.md)
· **Fonte:** `src/dsp/Hall.hpp`

---

## A ideia

O `SPACE` faz "eco que vira cauda" — mas não a densidade lisa de um FDN.
O `HALL` põe isso no patch: entra áudio, saem `L`/`R`, e `SIZE`/`DECAY`/
`DAMP` são o espaço. Como aceitam CV, um LFO no `SIZE` faz a sala
"respirar" (e desafina a cauda — efeito de fita), um `ENVELOPE` no `MIX`
faz a reverberação nascer com a nota, e o gate `FREEZE` sustenta um
acorde para sempre.

## Por dentro

**O que é uma "rede de atraso realimentada" (FDN), e por que soa mais
denso que o `SPACE`:** em vez de uma única linha de atraso
realimentando só a si mesma (o `SPACE`, #10), o `HALL` tem **8** linhas
de atraso de comprimentos diferentes, e a cada volta **todas** se
misturam entre si antes de cada uma retornar pro seu próprio início —
o eco de uma linha alimenta as outras 7 também. Como as 8 linhas têm
tempos diferentes (não múltiplos exatos umas das outras), as
repetições nunca se alinham de forma audível — o resultado, mesmo
sendo matematicamente feito de ecos discretos, soa como uma nuvem lisa
e contínua desde o início, sem precisar de nenhum estágio de difusão
separado.

**O que é a "matriz de Householder", e por que garante estabilidade
"pra qualquer `DECAY`":** misturar 8 sinais de qualquer jeito poderia
fazer a energia total **crescer** a cada volta (um risco real de
qualquer sistema realimentado, como visto no `CABEAMENTO.md` §1 sobre
feedback). A matriz de Householder é uma forma específica de misturar
que é **ortogonal** — uma propriedade matemática que garante que a
energia total das 8 linhas **nunca aumenta** na mistura em si, não
importa a configuração. Isso significa que o único controle real sobre
"quanto a cauda dura" é o `DECAY` (que decide quanta energia cada linha
perde por volta) — a mistura em si nunca é a causa de uma explosão,
então o `HALL` é seguro em qualquer ajuste, sem precisar de um
limitador de segurança adicional no laço.

`DAMP` aplica um filtro passa-baixa dentro de **cada** linha, a cada
volta — a mesma física real de uma sala de verdade, onde o ar e as
superfícies absorvem frequências agudas mais rápido que graves, então
a cauda de uma reverberação real **escurece** progressivamente
conforme decai (não é uma escolha estética arbitrária, é a física do
som se propagando e perdendo energia no ar).

**`PRE` (pré-atraso), por que ele define o "tamanho percebido"
independente do `DECAY`:** numa sala real, existe sempre um pequeno
intervalo **de silêncio** entre o som direto chegar aos seus ouvidos e
as primeiras reflexões da parede/teto chegarem — quanto **maior** essa
sala, mais longe as superfícies estão, e maior esse intervalo. O
ouvido usa esse intervalo (não a duração da cauda em si) como pista
primária de "quão grande é este espaço" — por isso `PRE` sozinho, sem
mexer em `DECAY`/`SIZE`, já muda a sensação de tamanho: separar o som
direto da reverberação por mais tempo soa como um espaço maior, mesmo
que a cauda em si seja idêntica.

## Os jacks, um a um

### Entradas

- **`IN`** (áudio, somado mono) — o que reverberar. **Plugue aqui:** a
  soma de várias vozes (por um `MIXER`), uma voz.
- **`SIZE`** (controle) — CV que soma ao `SIZE`. **Plugue aqui:** um LFO
  lento (a sala respira), `DRIFT.a`.
- **`DEC`** (decay) (controle) — CV que soma ao `DECAY`. **Plugue
  aqui:** um `ENVELOPE`, um `SEQUENCE` de CV (RT60 por seção).
- **`FRZ`** (freeze) (controle, gate) — enquanto alto, a cauda vira
  infinita (a rede preserva energia) e a entrada nova para. **Plugue
  aqui:** `CLOCK.euclid`, `DECISION.gate`, `HARMONY.change`.

### Saídas

- **`L`** / **`R`** (áudio) — canais esquerdo e direito,
  descorrelacionados (imagem larga). Vão a dois canais do `MIXER` com
  pans opostos, ou `L` → `MIXER`, `R` → outro processador.

## Os controles, um a um

**SIZE** (0–1) — o tamanho do espaço (~8 ms sala a ~88 ms hall).

**DECAY** (0–1) — o tempo de cauda (RT60: 0,2 s a 15 s). Perto do máximo
a cauda quase não decai.

**DAMP** (0–1) — o passa-baixa no laço. 0 = brilhante; 1 = escuro (o
agudo decai antes).

**MOD** (0–1) — a profundidade da modulação do ponto de leitura — chorus
na cauda, quebra o "apito" metálico. Determinístico.

**PRE** (0–1) — o pré-atraso (0 a ~120 ms). Separa o som direto da
reverberação — o que define o tamanho percebido.

**MIX** (0–1) — seco ↔ molhado. 0 = passa-direto.

## Como cabear

**Reverb de barramento:**
```
todas as vozes → MIXER → HALL (IN)
HALL (L) → MIXER (ch3)   (pan esquerda)
HALL (R) → MIXER (ch4)   (pan direita)
```

**Pad congelado infinito:**
```
CHORD (OUT) → HALL (IN)     MIX alto
gate longo → HALL (FRZ)     (o acorde vira drone enquanto o gate segura)
```

## Potencializar

- **Sala que respira:** um `FUNCTION` bem lento no `SIZE` — a
  reverberação muda de tamanho e a cauda desafina (efeito de fita).
- **Reverb que nasce na nota:** `ENVELOPE.env → MIX` (via `CONTROL`) —
  seco no ataque, molhado na cauda.
- **Freeze como seção:** `HARMONY.change → FRZ` momentâneo — a cada
  modulação de tom, um "flash" de reverb infinito.
- **Damp dinâmico:** `NOISE.smooth` lento no `DAMP` — a sala fica ora
  brilhante, ora abafada, ao longo de minutos.

## Se você conhece o Eurorack

Faz o papel de um reverb FDN de rack (NE Desmodus Versio, Strymon
StarLab, Erica Black Hole). A base é o FDN de Jot & Chaigne (1991), a
matriz de Householder/Hadamard, e o damping no laço de Dattorro (1997).
O parente "eco → cauda" é o `SPACE` (#10).
