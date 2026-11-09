# DYNAMICS — compressor e gate com sidechain

Implementação local de 8/11/2026, posterior à release 0.1.7, aprovada pelo
autor a partir do estudo do Cardinal
([`ESTUDO_cardinal_catalogo_e_licencas.md`](ESTUDO_cardinal_catalogo_e_licencas.md) §5.1).
Fonte: [Dynamics.hpp](../src/dsp/Dynamics.hpp). Família TRANSFORM; 16 HP.
Licença autoral: AGPL-3.0-or-later.

A lacuna mais visível do catálogo: o Modular só limitava no MASTER, e a
dinâmica dependia de combinações (ENVELOPE, SCOPE). O DYNAMICS abaixa o que
passa do limiar — ou fecha o que fica abaixo — e devolve o que ouve como
sinal de controle.

## Contrato

**Entradas.** IN (áudio, mono ou estéreo; os canais são detectados juntos —
o detector ouve o maior módulo entre eles). SIDE (qualquer sinal; o botão SIDE
cruza o que o detector ouve entre IN e SIDE; sem o jack SIDE ligado, ouve IN). THR
(CV somada ao limiar, −1..+1 = −24..+24 dB, limitado a −72..+6 dB).

**Saídas.** OUT = IN × ganho (redução + GAIN), limitado a ±4 e zerado se
não finito. ENV = o nível do detector em 0–1 (−60..0 dB), com a mesma
balística de ATK/REL. GR = a redução atual em 0–1 (0..24 dB).

**Detector.** Pico (|x| por amostra) ou RMS (média quadrática de ~10 ms,
escalada por √2 para que um seno leia o seu pico).

**Curva estática** (dB, Giannoulis/Massberg/Reiss): compressor — abaixo de
T − W/2 nada; no joelho, −(1 − 1/R)(x − T + W/2)²/2W; acima, −(1 − 1/R)(x − T).
RATIO ≥ 19,5 é limitador (1/R = 0). Gate (expansor para baixo) — acima de
T + W/2 nada; no joelho, −(R − 1)(W/2 − (x − T))²/2W; abaixo, (R − 1)(x − T),
até −80 dB.

**Balística** no domínio do ganho: ataque quando a redução aumenta, release
quando diminui; coeficientes de um polo (1 − e^(−1/(fs·t))).

**Desvios Rasgo.** (a) ENV e GR como sinais. (b) BRTH: golpe = o nível
cruza o limiar para cima depois de ≥ 30 ms abaixo; o intervalo entre golpes
(média móvel, peso 0,35) leva o release para clamp(0,6 × intervalo,
20 ms, 2 s), na proporção de BRTH. (c) DRIFT: a cada golpe, ataque e
release são multiplicados por 2^(u·DRIFT), u uniforme em [−1, 1) de um
xorshift semeado no `prepare()` — mesma entrada, mesma saída.

**Tempo real.** `process()` não aloca; NaN/∞ na entrada viram zero; o
contador entre golpes tem teto (sem estouro em sessões longas).

**Na seed desde 9/11** (decisão do autor: "por direto no sorteio, mais
simples"; até 8/11 ficava fora, como LISTEN e PHRASE). Entrou no catálogo da
seed (TRANSFORM, no fim) e na cadeia de processadores do `seedPatch` (6
opções em vez de 5). Por isso **as seeds anteriores a 9/11 geram outros
patches**. Na cadeia, a compressão é musical: limiar −30..−12 dB, ratio 2–6,
ataque 2–30 ms, release 60–400 ms, joelho 3–10 dB, GAIN que devolve só 35 %
do que a curva tira (até 12 dB), RMS em metade das vezes, BRTH em 35 % e
DRIFT em 30 %. Em 35 % das vezes, o `DRUM` vai ao SIDE (o pad que respira).
O GAIN (`makeup`) fica fora do passeio e da mutação, e no VARIA é tratado
como nível. Sondagem de 1000 seeds: DYNAMICS com IN ligado em 253, com SIDE
em 173.

## Testes

`tests/test_dynamics.cpp`: curva estática (dura, joelho, limitador, gate),
seno de −6 dB comprimido em 9 dB (4:1, limiar −18) e devolvido pelo GAIN,
tempos de ataque e release, gate fechando o que está abaixo, ducking pelo
SIDE (~19 dB durante o golpe), SIDE = 0 sem efeito, ENV/GR se movendo
sem IN, CV no limiar, BRTH soltando entre golpes (GR 0,17 contra 0,70 com
release fixo de 2 s), DRIFT determinístico e diferente de zero, robustez a
NaN/∞.

## Proveniência e limites

Formulação pública: D. Giannoulis, M. Massberg, J. D. Reiss, "Digital
Dynamic Range Compressor Design — A Tutorial and Analysis", *JAES* 60(6),
2012. Conceito lido (sem copiar código) no Bogaudio PRESSOR (GPL-3.0+), no
ZamComp de Damien Zammit (GPL-2.0+) e no CVfunk Pressed Duck de Cody Geary
(MIT), dentro do Cardinal 26.02.

Sem *lookahead* (o golpe passa antes da redução se o ataque for maior que
ele), sem detecção por banda e sem estéreo desligado (os canais compartilham
o ganho). A escuta do autor ainda não foi feita.
