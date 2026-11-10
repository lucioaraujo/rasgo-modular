# Proposta — os quatro canais da seed (conjunto, papéis e palco)

Data: 10/11/2026. Estado: **implementado em 10/11** ("avance" do autor),
com os valores deste desenho: distribuição do §2, ARP/MARKOV ~meio a meio
(o registro decide quando só um cabe), pans **no centro** — as casas do §3
estão no código atrás de `kSeedOpensStage` (desligado) até a decisão do
autor. Falta o teste de máscara por registro medido (§5).
Pedido do autor: "acho interessante que hajam mais seeds utilizando os
quatro canais, sobretudo para resultados polifônicos e mais espacialidades
de pan". Condição: "não há problema de mudar a composição de todas as
seeds. só não desejo que haja problemas técnicos de qualidade de som".

## 1. Hoje (medido em 1000 seeds)

| canais | seeds |
|---|---|
| 1 | 48 % |
| 2 | 34 % |
| 3 | 18 % |
| 4 | 0 % |

Canal 1 = a voz (espinha). Canais 2–3 = "camadas extras": metade das seeds,
sempre fontes já processadas, entre −24 e −11 dB. O canal 4 nunca é usado.
Nenhuma camada tem **altura própria** — polifonia de verdade não existe.

Restrição do rack: **um módulo de cada tipo**. A contravoz usa um módulo
diferente do da voz; só há um QUANTIZER, que é da voz.

## 2. O tamanho do conjunto vira gene da seed

Um gene novo `ensemble` na identidade da seed (fluxo próprio, o 16 — os
outros genes não se deslocam), ligado à complexidade:

| vozes | seed simples | média | densa |
|---|---|---|---|
| 1 | 35 % | 15 % | 5 % |
| 2 | 35 % | 30 % | 20 % |
| 3 | 20 % | 30 % | 35 % |
| 4 | 10 % | 25 % | 40 % |

(em média ~20 / 30 / 30 / 20 %)

## 3. Um papel por canal

| canal | papel | o que soa | casa no palco |
|---|---|---|---|
| 1 | **voz** | a espinha de hoje | centro (0) |
| 2 | **contravoz** | segunda linha com altura própria | ±0,30 |
| 3 | **textura** | as camadas de hoje (processadas, baixas) | ∓0,45 |
| 4 | **eco ou pulso** | o corpo da voz num espaço, ou a percussão | oposto à contravoz, ±0,40 |

Com 2 vozes: voz + contravoz. Com 3: + textura. Com 4: + eco/pulso. Lado
(esquerda/direita) sorteado por seed; nenhuma casa passa de 0,55 (a abertura
aprovada). Grave sempre no centro (já no MIXER).

### 3.1 A contravoz (canal 2) — a polifonia

**De onde vêm as notas** — dois caminhos, sorteados:

- **ARP** (50 %): a raiz do arpejo é a nota da voz (`QUANTIZER.pitch →
  ARP.root`), num relógio próprio (o CLOCK em outra divisão, ou a LOGIC):
  notas do acorde **em torno** da melodia — contraponto harmônico.
- **MARKOV** (50 %): aprende as transições da voz (`QUANTIZER.pitch/gate →
  LRN`) e improvisa no seu próprio relógio — **imitação**: uma segunda
  linha que fala a mesma língua da primeira, sem repeti-la. Como aprende só
  notas da escala, fica na escala.

**Quem canta** (um módulo diferente do da voz), sempre com envelope próprio
e nunca cru no MIXER:

- **STRING** ou **MATTER** (dedilhado/golpe pelo gate da contravoz) — já
  têm corpo e decaimento;
- ou **PULSAR / WAVETABLE / ADDITIVE / OPERATOR → LPG** (golpe pelo gate) —
  o LPG é o processamento e o envelope.

**Registro**: uma oitava acima ou abaixo da voz (sorteado), para as duas
linhas não se colarem nem se mascararem. **Nível**: 4 a 8 dB abaixo da voz.

### 3.2 A textura (canal 3)

Como hoje: só fontes já processadas (FILTER/WASP/LPG/SPACE/PARAMETRIC),
−24 a −11 dB.

### 3.3 Eco ou pulso (canal 4)

- **Eco (60 %)**: a saída `wet` de um espaço sobre a voz (SPRING, PRISM,
  HALL, SPACE, MEMORY ou LOOPER) — o corpo da voz num lugar do palco,
  separado do som seco.
- **Pulso (40 %)**: o DRUM, quando não estiver na espinha.

## 4. Densidade: mais vozes, menos notas cada

Com 3–4 vozes, a densidade de cada relógio cai (a contravoz numa divisão
mais lenta que a voz; a textura mais rarefeita). É a ideia do orçamento de
eventos (BUDGET, Antitotem) em versão leve, para não ficar tudo cheio ao
mesmo tempo.

## 5. Qualidade técnica — a condição do autor

Toda mudança passa por `tests/test_seed_quality.cpp` (feito em 10/11):
24 seeds × 6 s, reprova não-finito, pico acima do teto, limitador segurando
> 5 % do tempo, DC, agudo acima de 8 kHz > −15 dB, e volume fora da faixa.
Acrescento a este teste:

- **nenhuma fonte crua** (OSC, PLL, CHORD, NOISE, FUNCTION, CHAOS) direto
  num canal do MIXER;
- **máscara**: com contravoz, voz e contravoz não podem estar no mesmo
  registro (diferença média ≥ 7 semitons);
- a régua roda também com **4 vozes forçadas**, o pior caso de soma.

E o equilíbrio de volume da seed acerta o total como hoje.

## 6. O que fica para depois

- O PALCO do VARIA e as entradas PAN do MIXER passam a usar estas casas.
- Regras de contraponto mais finas (movimento contrário etc.) — só se a
  escuta pedir; começo leve.

## 7. Decisões para o autor

1. A distribuição do §2 serve (média ~20/30/30/20)?
2. A seed já abre os canais nas **casas** do §3 (pans fora do centro), ou
   continua no centro e só o PALCO/PAN os move?
3. Contravoz: ARP e MARKOV meio a meio, ou preferência por um?
