# Receitas de patch

Dez montagens completas, passo a passo: a lista de módulos, a ordem de
cabeamento, os valores de partida, e o que ouvir enquanto você ajusta.
Cada uma parte do zero (rack vazio ou um seed qualquer) e chega a um
resultado reconhecível. Nomes de jack e faixas de valor conferem com as
páginas de módulo — linkadas em cada receita pra quem quiser o
mecanismo por trás.

Se você ainda não leu [`COMO_PENSAR.md`](COMO_PENSAR.md) e
[`CABEAMENTO.md`](CABEAMENTO.md), vale ler antes: as receitas assumem
que você já sabe que saída vai numa entrada, que o `MIXER`/`MASTER` são
o fim da linha, e o que é um gate × uma CV.

---

## 1. Voz subtrativa completa

A cadeia mínima de um sintetizador — a base de quase tudo. Ver
[`18_oscilador.md`](18_oscilador.md), [`02_filtro.md`](02_filtro.md),
[`06_envelope.md`](06_envelope.md).

**Módulos:** `CLOCK`, `SEQUENCE`, `QUANTIZER`, `OSC`, `FILTER`,
`ENVELOPE`, `MIXER`.

1. `CLOCK.CLK` → `SEQUENCE.CLK` (o passo anda no relógio).
2. `CLOCK.EUC` → `ENVELOPE.GATE` (o ritmo dispara a nota).
3. Escreva um riff simples em `SEQUENCE.P1`–`P8` — um contorno que sobe
   e desce (ex.: 0, 0.2, 0.4, 0.2, 0, −0.2, 0, 0.4). `MODE` = 0 (pra
   frente).
4. `SEQUENCE.PTCH` → `QUANTIZER.CV`. `SCALE` = maior, `ROOT` = C.
5. `QUANTIZER.PTCH` → `OSC.1V/O`.
6. `OSC.SAW` → `FILTER.IN`.
7. `FILTER.ALL` → `ENVELOPE.IN`.
8. `ENVELOPE.OUT` → `MIXER.ch1`.
9. Bônus: `ENVELOPE.ENV` → `FILTER.FC` — o filtro abre com cada nota.

**Valores de partida:** `FILTER.CUT` ~800 Hz, `RESO` ~0,3; `ENVELOPE`
`ATK` 5 ms, `DEC` 200 ms, `SUS` 0,3, `REL` 300 ms.

**O que ouvir:** suba `CUT` até a voz ficar clara sem estridência; suba
`RESO` até sentir o filtro "cantar" perto da nota, sem chegar a
auto-oscilar. Se a nota ficar "presa demais", baixe `SUS` — em 0 vira
um *pluck*.

**Variação:** troque `SAW` por `PLS` (com `PW` variando) pra um timbre
mais oco; troque o `OSC` pelo `MATTER`/`STRING` pra uma voz com corpo
físico em vez de eletrônica.

---

## 2. Montando um "Maths"

O Make Noise Maths é uma caixa de ferramentas de modulação — dois
canais de função, atenuversão/soma, e lógica de comparação. No Rasgo
essas três coisas moram em módulos separados; junte-os. Ver
[`01_gerador_de_funcao.md`](01_gerador_de_funcao.md),
[`21_control.md`](21_control.md), [`22_logic.md`](22_logic.md).

**Módulos:** `FUNCTION` ×2, `CONTROL`, `LOGIC`, `CLOCK`.

1. **Canal 1 — LFO livre:** `FUNCTION #1`, `RATE` ~0,2 Hz, `SLOPE` 0,5
   (triângulo), `SYNC` desligado — corre sozinho, sem gate.
2. **Canal 2 — envelope disparado:** `CLOCK.EUC` → `FUNCTION #2.SYNC`
   (chave `SYNC` ligada), `SLOPE` = 0 (serra ↓) — cada pulso reinicia
   uma rampa que decai, um AD simples.
3. **Somar os dois, com pesos:** `FUNCTION #1.BI` → `CONTROL.IN1`
   (`SCALE1` ~0,6); `FUNCTION #2.UNI` → `CONTROL.IN2` (`SCALE2` ~−0,4,
   invertido). `CONTROL.SUM` → `FILTER.FC`.
4. **Lógica — um gatilho de coincidência:** `CLOCK.EUC` → `LOGIC.A`;
   `TRIGSEQ.T1` (ou outro gate qualquer do patch) → `LOGIC.B`.
   `LOGIC.AND` só dispara quando os dois coincidem — cabeie num
   `ENVELOPE.gate` de acento.

**O que ouvir:** o `FILTER.FC` recebe **dois** movimentos ao mesmo
tempo — o LFO lento e contínuo, mais o solavanco do envelope disparado
— e o atenuversor negativo no canal 2 faz esse solavanco **puxar o
corte pra baixo** em vez de pra cima. É exatamente o tipo de mistura
que o Maths faz com a saída `SUM`.

**Variação:** troque o `AND` por `XOR` no passo 4 — o gatilho passa a
disparar só quando exatamente um dos dois clocks acontece, nunca os
dois juntos (sincopação).

---

## 3. Percussão generativa

Um beat de verdade a partir de um clock e um groove — sem desenhar
nenhum passo. Ver [`05_clock.md`](05_clock.md),
[`30_trigseq.md`](30_trigseq.md), [`47_drum.md`](47_drum.md).

**Módulos:** `CLOCK`, `TRIGSEQ`, `DRUM` ×3, `LOGIC`, `MIXER`.

1. `CLOCK.CLK` → `TRIGSEQ.CLK`.
2. `TRIGSEQ.T1` → `DRUM #1.GATE` — `TONE` grave (~50 Hz), `BEND` alto
   (bumbo).
3. `TRIGSEQ.T2` → `DRUM #2.GATE` — `TONE` médio, `SNAP` alto (caixa).
4. `TRIGSEQ.T3` → `DRUM #3.GATE` — `TONE` agudo, `DECAY` curto, `MAP`
   alto (chimbal).
5. `TRIGSEQ.ACC` → `DRUM #1.ACC` (o bumbo acentua nos golpes fortes).
6. `DRUM #1/#2/#3.OUT` → `MIXER.ch1/ch2/ch3`.
7. **Virada automática:** `CLOCK.CLK` → `LOGIC.CLK` com `LOGIC.DIV` = 8
   (via `LOGIC.DIV` → `TRIGSEQ.FILL`) — a cada 8 compassos, as
   densidades sobem sozinhas por um instante.

**Valores de partida:** `TRIGSEQ.MAP` ~0,3 (entre reto e quebrado),
`DNS1` (bumbo) ~0,7, `DNS2` (caixa) ~0,4, `DNS3` (chimbal) ~0,6, `CHAOS`
~0,1 (humaniza sem desmontar o groove), `RATCH` ~0,1.

**O que ouvir:** gire `MAP` devagar de 0 a 1 com tudo tocando — o
caráter do beat inteiro muda (reto → quebrado → suingado → esparso)
sem nenhum passo mudar de posição. Suba `CHAOS` até notar as primeiras
notas-fantasma; é fácil passar do ponto — volte um pouco.

**Variação:** `TRIGSEQ.MAP` recebendo um `FUNCTION` bem lento faz o
groove evoluir sozinho ao longo da peça, sem repetir.

---

## 4. Drone espectral

Um pad infinito a partir de qualquer coisa — aqui, de um trem de
grãos. Ver [`56_pulsar.md`](56_pulsar.md),
[`57_spectra.md`](57_spectra.md).

**Módulos:** `PULSAR`, `SPECTRA`, `HALL`, `MIXER`.

1. `PULSAR` sozinho, sem `PIT` cabeado — `FREQ` ~90 Hz, `FORMANT` ~2,
   `WINDOW` ~0,4 (Hann) — já toca uma textura pulsante.
2. `PULSAR.L` → `SPECTRA.IN`.
3. `SPECTRA.VOICES` alto (~20), `BLUR` alto (~0,8), `STRETCH` ~0,3 (um
   leve caráter inarmônico).
4. `CLOCK.EUC` (bem devagar, `RATE` baixo) → `SPECTRA.FRZ` — o
   espectro congela num instante e vira drone sustentado.
5. `SPECTRA.L`/`R` → `HALL.IN` → `MIXER.ch1`/`ch2` (via `HALL.L`/`R`).

**O que ouvir:** com `BLUR` alto, o espectro "arrasta" atrás do
`PULSAR` — cada vez que você `FREEZE`, ele para exatamente onde estava
e vira um pad estático; solte o `FREEZE` e ele volta a seguir a fonte.

**Variação:** troque o `PULSAR` por qualquer voz — uma frase de
`CHORD`, um trecho de `SIGNAL-IN` — e congele no instante que quiser
guardar.

---

## 5. Eco que vira sala

O mesmo módulo, do *slapback* ao *hall*, sem trocar de nada — a
demonstração central do [`10_space.md`](10_space.md).

**Módulos:** qualquer voz, `SPACE`, `MIXER`.

1. voz → `SPACE.IN`.
2. Comece com `TAPS` = 1, `SPREAD` = 0, `DIFFUSION` = 0, `FEEDBACK` ~0,3
   — um eco limpo e discreto.
3. Suba `FEEDBACK` e `DIFFUSION` **juntos**, devagar, com a mão nos dois
   knobs ao mesmo tempo (`DIFFUSION` não tem entrada de CV — este passo
   é manual, não automatizável por cabo).
4. Em algum ponto no meio do caminho, o eco discreto já começou a
   borrar; perto do topo dos dois, virou uma cauda contínua de sala.

**O que ouvir:** não existe um ponto exato de "virou reverb" — é um
gradiente contínuo. Pare onde soar certo pro patch: eco rítmico
(`DIFFUSION` baixo) pra uma linha percussiva, sala cheia
(`DIFFUSION`/`FEEDBACK` altos) pra colar um pad.

**Variação:** suba `TAPS` pra 5 e gire `SPREAD` de 0 a 1 — o eco único
se abre num padrão rítmico antes mesmo de mexer em `DIFFUSION`.

---

## 6. Corda tocada por acaso

Uma melodia que emerge do acaso e se fecha quando você decide — o par
[`08_turing.md`](08_turing.md) + [`11_string.md`](11_string.md).

**Módulos:** `CLOCK`, `TURING`, `QUANTIZER`, `STRING`, `SPACE`,
`MIXER`.

1. `CLOCK.CLK` → `TURING.CLK`.
2. `TURING.CV` → `QUANTIZER.CV`.
3. `CLOCK.EUC` → `QUANTIZER.TRIG` (a nota só atualiza no pulso — sem
   isto, a melodia tremularia fora de tempo).
4. `QUANTIZER.PTCH` → `STRING.1V/O`.
5. `CLOCK.EUC` → `STRING.PLK` (o mesmo pulso pinça a corda).
6. `STRING.OUT` → `SPACE.IN` → `MIXER.ch1`.

**Valores de partida:** `TURING.LEN` ~8, `LOCK` ~0,4 (mais acaso que
memória), `MUTATE` ~0,3. `STRING.DECAY` médio, `DAMP` médio.

**O que ouvir:** com `LOCK` em 0,4 a linha muda a cada volta. Suba
`LOCK` devagar enquanto escuta — quando um trecho de 8 notas soar bem,
continue subindo até perto de 1: o laço fecha e repete exatamente
aquele trecho.

**Variação:** desça `LOCK` de volta a 0 no meio de uma frase pra
"soltar" o laço de novo — abrir e fechar o `LOCK` ao vivo é o gesto
central deste módulo.

---

## 7. Barber-pole

O glissando infinito de Shepard-Risset — ver a explicação completa em
[`58_shifter.md`](58_shifter.md).

**Módulos:** um pad ou drone qualquer, `SHIFTER`, `MIXER`.

1. pad → `SHIFTER.IN`.
2. `SHIFTER.SHIFT` ~8 Hz (pequeno — a chave do efeito).
3. `SHIFTER.FEEDBACK` ~0,8 (positivo = sobe pra sempre; negativo = desce
   pra sempre).
4. `SHIFTER.UP` → `MIXER.ch1`, `MIX` ~0,6.

**O que ouvir:** o espectro parece subir sem nunca "chegar" a lugar
nenhum — se `FEEDBACK` estiver baixo, o efeito é sutil (quase um
*phasing* lento); perto de 0,9 fica bem óbvio. Cuidado: valores
extremos podem levar tempo pra estabilizar — dê alguns segundos.

**Variação:** troque `UP` por `DN` no passo 4 pra uma queda infinita em
vez de subida — bom em transições/quedas de seção.

---

## 8. O patch que evolui sozinho

Composição por ausência de gesto — decidir o que deriva e deixar
rodar. Ver [`27_drift.md`](27_drift.md) e
[`COMO_PENSAR.md`](COMO_PENSAR.md) §6.

**Módulos:** qualquer seed, `DRIFT` (se não vier no seed), o toggle
`VARIA` do cabeçalho.

1. Abra um seed qualquer (`⚄ SEED` ou um número específico).
2. Se não houver um `DRIFT` já cabeado, adicione um e ligue `DRIFT.A`,
   `.B`, `.C` em três parâmetros **estruturais** — `FILTER.FC`,
   `SPACE.FBK`, `CHORD.CHRD` são bons candidatos (não a altura da
   melodia — isso é o `HARMONY`/`QUANTIZER`).
3. `DRIFT.RATE` bem baixo (0,01–0,05 Hz — minutos por volta).
4. Não toque em mais nada. Deixe rodar de 3 a 5 minutos ouvindo.

**O que ouvir:** nos primeiros 30 segundos, quase nada muda — é
esperado, `DRIFT` opera em minutos. Depois de um tempo, o timbre e o
espaço já não são os mesmos, mas você não vai conseguir apontar **o
momento exato** da mudança — é o ponto.

**Variação:** ligue `VARIA` (o toggle do cabeçalho) por cima disso — ele
move **todo** o patch (menos `MIXER`/`MASTER`) na velocidade da energia
do próprio som, uma segunda camada de movimento, mais rápida e reativa
que o `DRIFT`.

---

## 9. O instrumento que se ouve

O sistema fechando o próprio laço — ver [`29_scope.md`](29_scope.md) e
[`COMO_PENSAR.md`](COMO_PENSAR.md) §8.

**Módulos:** um patch qualquer já tocando, `SCOPE`, `OSC`, `ENVELOPE`.

1. `MIXER.L+R` → `SCOPE.IN`.
2. `SCOPE.THRU` → `MASTER.IN` (substitua o cabo direto do `MIXER` por
   este — o `SCOPE` fica no caminho sem mudar o som).
3. `SCOPE.ONS` → `ENVELOPE.GATE` — um envelope novo dispara a cada
   ataque/transiente da mistura inteira.
4. `SCOPE.PIT` → `OSC.1V/O` — um oscilador extra afina "de ouvido" pela
   altura predominante da mistura.
5. `ENVELOPE.OUT` do passo 3 e a saída do `OSC` do passo 4 → canais
   livres do `MIXER`.

**O que ouvir:** o `OSC` novo persegue a altura do que já está tocando
— em uma voz monofônica limpa ele afina de perto; numa mistura densa,
ele "erra" e pula, o que também é musicalmente interessante. `SENS` do
`SCOPE` controla quão sensível o `ONS` é a transientes pequenos.

**Variação:** `SCOPE.LVL` (o pico) invertido (via `CONTROL`) no `VCA`
de um pad é um *ducking* automático — o pad cede espaço quando o resto
da mistura fica cheio.

---

## 10. Vocoder falado

A voz robô clássica, com uma voz de verdade. Ver
[`60_vocoder.md`](60_vocoder.md) e [`49_signal_in.md`](49_signal_in.md).

**Módulos:** `OSC`, `SIGNAL-IN`, `VOCODER`, `MIXER`.

1. `OSC.SAW` → `VOCODER.CAR` (a portadora — precisa ser rica em
   harmônicos; uma serra é o clássico).
2. `SIGNAL-IN.L` → `VOCODER.MOD` (fale ou cante perto do microfone
   capturado pelo sistema).
3. `VOCODER.BANDS` alto (16–20) pra fala inteligível; `SIBILANCE` ~0,5
   pra não perder os "s"/"f".
4. `VOCODER.OUT` → `MIXER.ch1`.
5. Opcional — tocar a portadora ao vivo: `SIGNAL-IN.1V/O` →
   `OSC.1V/O` (um teclado MIDI conectado toca a nota da voz robô
   enquanto você fala/canta o ritmo e o texto).

**O que ouvir:** com `BANDS` baixo (6–8), o efeito soa mais "robô dos
anos 70", grosso; subindo pra 16–20, a fala fica reconhecível. Se as
consoantes sumirem, suba `SIBILANCE`.

**Variação:** troque a portadora por um `CHORD` — a voz passa a "falar"
um acorde inteiro, não uma nota só.

---

Consulte o [apêndice de equivalências](APENDICE_equivalencias.md) pra
comparar cada peça com o módulo/técnica equivalente fora do Rasgo.
