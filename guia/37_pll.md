# PLL — oscilador de malha de fase

**Família:** SOURCE · **Módulo 37**
**Essência:** um segundo oscilador que **persegue** uma referência de
fase em vez de resetar duro — trava em sub e super-harmônicos, tem uma
rede de feedback selecionável, e uma saída que diz quão travado está.
**Dossiê técnico:** [`../dossies/37_pll.md`](../dossies/37_pll.md)
· **Fonte:** `src/dsp/Pll.hpp`

---

## A ideia

O `OSC` tem hard sync — a fase reinicia de um golpe num mestre. O `PLL`
faz o oposto: quando você liga uma referência (`REF`), ele **curva a
própria taxa** aos poucos para acompanhar a fase da referência, como um
músico se ajustando a outro. Solto (sem `REF`), é um oscilador de
verdade, com afinação própria. É o "segundo oscilador sofisticado" —
não um modo do `OSC`.

## Por dentro

**O que é um "detector de fase" e por que a malha não trava
instantaneamente:** a cada instante, o `PLL` compara **onde ele está**
na própria volta (ver "Por dentro" do `18_oscilador.md` — a mesma ideia
de fase) com onde a referência `REF` está na dela, e calcula um "erro":
quão longe as duas estão de estarem sincronizadas. Em vez de saltar pra
zerar esse erro na hora (o que seria um hard sync, como o `OSC`), o
`PLL` usa esse erro pra **curvar aos poucos** a própria taxa — acelera
um pouco se está atrasado, desacelera se está adiantado — até o erro
chegar perto de zero. `LOCK_GAIN` é **o quanto** essa curva reage ao
erro: fraco, a correção é lenta e você ouve o oscilador "escorregando"
até encontrar a referência (o "caçar"); forte, a correção é rápida, mas
fica mais sensível a instabilidade se a razão pedida não for exata.

**Por que o alcance de captura é limitado (não trava em qualquer
razão):** a força de correção só consegue empurrar a taxa até certo
ponto por unidade de tempo. Se a razão que você pede (via `RATIO`) está
**longe demais** do que a referência realmente oferece, o erro nunca
diminui rápido o bastante antes da fase já ter derivado de novo — o
PLL fica preso "correndo atrás" sem nunca alcançar, um comportamento
real de qualquer malha de fase (não uma limitação exclusiva daqui) —
esse "caçar sem travar" é audível como um glissando instável, às vezes
o efeito desejado.

`RATIO` multiplica a fase de `REF` **antes** da comparação — então
"sincronizar" pode significar "uma volta pra cada duas da referência"
(oitava acima) ou "uma volta a cada oito" (um divisor bem fundo), não
só "uma volta pra cada volta".

**`RING`, o heterodino:** multiplicar duas ondas periódicas (aqui,
`out × ref`) é a mesmíssima operação de `RingMod` explicada em
`RELACAO_DE_CABO.md` §1.1 — o resultado carrega as frequências soma e
diferença das duas, não as originais. Aqui ele nasce pronto como uma
terceira saída, sem precisar de nenhum cabo com relação.

**`FTYP`/`FB` — por que isso não desafina:** essa rede não toca na
**taxa** do acumulador de fase (o que mudaria a altura) — ela modula
**em que ponto exato da volta** a forma de onda é lida antes de sair
(distorção de fase, a mesma família de ideia por trás do `warp` do
`WAVETABLE`, #40). Deslocar onde você lê dentro do mesmo ciclo muda o
**formato** da onda (e portanto o timbre) sem mudar quantas voltas por
segundo ela dá — por isso `FB` acrescenta grão/textura sem tirar a
afinação do lugar.

## Os jacks, um a um

### Entradas

- **`1V/O`** (controle, altura) — multiplica `FREQ`. **Plugue aqui:**
  `QUANTIZER.pitch`, `SEQUENCE.pitch` — o PLL solto toca a melodia.
- **`FM`** (áudio) — modulação de frequência linear. **Plugue aqui:**
  outra voz (timbre) ou um LFO (vibrato). Intensidade no knob `FM`.
- **`REF`** (áudio) — a **referência de fase**. Espera uma **serra
  bipolar**. **Plugue aqui:** `OSC.saw` (de outro oscilador), qualquer
  saída periódica forte. Com `REF` conectado, o PLL persegue; sem, toca
  livre.

### Saídas

- **`OUT`** (áudio) — a forma de onda (`SHAPE`), na frequência já
  corrigida pela perseguição. Vai ao `MIXER` (via filtro/envelope).
- **`RING`** (áudio) — `out × ref` — o heterodino clássico entre dois
  osciladores (bandas soma e diferença). Fica em silêncio sem `REF`.
  Vai ao `MIXER`.
- **`LOCK`** (controle) — 0 a 1: quão perto o detector está de zero
  (1 = travado). **Plugue em:** `VCA.cv` (o volume sobe quando trava,
  cai quando caça), `FILTER.cutoff`, qualquer `_mod` — o patch reage ao
  estado de travamento.

## Os controles, um a um

**FREQ** (8–8000 Hz) — a frequência livre (sem `REF`) e o ponto de
partida (com `REF`).

**FINE** (±100 cents) — afinação fina.

**SHAPE** (0–3) — morph contínuo: seno → triângulo → serra → quadrada.
Uma saída de forma variável (o `OSC` tem 5 fixas).

**RATIO** (0,03–8×) — multiplica a fase de `REF` antes de comparar. O PLL
trava em sub/super-harmônicos: perto de 1 = uníssono; 2 = oitava acima;
perto de 0,03 = um divisor bem fundo da referência.

**LOCK_GAIN** (0–1) — força da perseguição. Baixo = "caça" devagar
(glissa até travar — audível); alto = trava rápido, mas fica mais
instável se a razão não bate exato.

**FM** (fm_amount, 0–1) — quantidade de FM linear pela entrada `FM`.

**FB** (feedback_amount, 0–1) — quanto da rede de feedback entra na fase
lida. Cria textura; não desafina.

**FTYP** (feedback_type, 0–5) — o tipo de feedback: direto · retificado ·
capacitivo (memória lenta) · pulso · "transistor" (tanh assimétrico) ·
refluxo.

## Como cabear

**Solto (oscilador comum):**
```
QUANTIZER → PLL (1V/O)
PLL (OUT) → FILTER (in) → ENVELOPE (in) → MIXER (ch1)
```

**Travando numa referência:**
```
OSC (saw) → PLL (REF)
PLL (OUT) → MIXER (ch2)     (um segundo oscilador que "orbita" o primeiro)
PLL (LOCK) → VCA (cv)       (o segundo oscilador some quando perde o lock)
```

## Potencializar

- **Glissando de captura:** `LOCK_GAIN` baixo e mude `RATIO` ao vivo —
  o PLL glissa de um harmônico ao outro tentando travar.
- **Barber-pole harmônico:** varra `RATIO` bem devagar com um `FUNCTION`
  — a segunda voz sobe e desce a escala de harmônicos da referência.
- **Textura por feedback:** com o PLL travado, suba `FB` e passeie por
  `FTYP` — o timbre ganha grão sem sair da afinação.
- **`RING` como voz:** o heterodino `out × ref` é uma terceira fonte —
  cabeie no seu próprio canal do `MIXER`.

## Se você conhece o Eurorack

Não há equivalente direto comum. É um PLL de VCO — o conceito aparece em
alguns módulos de rádio/experimentais e no Doepfer A-196. A base é o
`OSC5` do ANTITOTEM (código do autor). O alcance de captura finito é
uma limitação real de PLL (documentada), não um bug — e musicalmente é o
"caçar" que dá o gesto.
