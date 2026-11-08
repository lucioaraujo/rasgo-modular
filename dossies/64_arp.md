# ARP — arpejador

Implementação local de 8/11/2026, posterior à release 0.1.7, aprovada pelo
autor a partir do estudo do Cardinal
([`ESTUDO_cardinal_catalogo_e_licencas.md`](ESTUDO_cardinal_catalogo_e_licencas.md) §5.2).
Fonte: [Arp.hpp](../src/dsp/Arp.hpp). Família TIME; 16 HP.
Licença autoral: AGPL-3.0-or-later.

O Modular tinha quem fizesse as notas de um acorde (CHORD, HARMONY) e quem
fizesse o tempo (CLOCK, SEQUENCE, TRIGSEQ), mas ninguém que percorresse as
notas de um acorde no tempo. O ARP faz isso, e soa sozinho.

## Contrato

**Entradas.** CLK (borda de subida = um passo; livre → relógio interno em
RATE). RST (o próximo passo volta ao começo do ciclo). ROOT (1 V/oct somado
à altura; aceita direto a saída `root` do HARMONY, que é semitom/12). CHD
(CV somada ao CHORD: cada 1/9 = um formato).

**Saídas.** PTCH = ROOT + semitons/12 da nota atual. GT = gate de cada
nota, com GATE × período (o período do clock externo é medido entre
pulsos; o interno é 1/RATE); nada nos passos pulados. EOS = pulso de 5 ms
no começo de cada ciclo.

**Notas.** Os 10 formatos do CHORD (uníssono, oitavas, quinta, maior,
menor, sus4, maj7, min7, dim, add9), sem repetir notas iguais; INV (0–3)
sobe as mais graves uma oitava; OCT (1–4) repete o acorde nas oitavas de
cima — até 16 notas, em ordem crescente.

**Modos (MODE 0–7).** Sobe; desce; ida e volta (2n − 2 passos); ida e
volta repetindo as pontas (2n); aleatório; embaralhado (uma permutação
nova a cada ciclo, Fisher–Yates); divergir (centro, um acima, um abaixo,
dois acima…); convergir (o inverso).

**Desvios Rasgo.** (a) divergir/convergir; (b) SKIP — em cada passo, com
probabilidade SKIP, o gate não abre (a altura avança mesmo assim); (c) ROOT
compatível com o HARMONY. Aleatório, embaralhado e SKIP usam um xorshift
semeado no `prepare()`: o mesmo patch toca a mesma sequência.

**Tempo real.** `process()` não aloca; NaN/∞ nas entradas viram zero;
parâmetros fora da faixa são limitados.

**Fora do catálogo da seed** (como LISTEN, PHRASE e DYNAMICS).

## Testes

`tests/test_arp.cpp`: notas dos formatos (sem repetir, inversão, 4
oitavas), os oito modos, embaralhado como permutação por ciclo, aleatório e
embaralhado determinísticos, relógio interno sem nada ligado (7 notas no
primeiro segundo a 6 Hz), clock externo e duração do gate, EOS nos
começos de ciclo, ROOT do HARMONY (+7/12 = uma quinta), CHORD por CV,
RESET, SKIP (~metade dos passos a 0,5, igual em duas rodadas), robustez.

## Proveniência e limites

Ideia pública: os arpejadores de sintetizador dos anos 1970 (Roland
Jupiter-4, ARP Odyssey) e os seus modos clássicos. Conceito lido, sem copiar
código, no Bogaudio ARP (GPL-3.0-or-later) e no Arp 3.x da Amalgamated
Harmonics (BSD-3-Clause), dentro do DISTRHO Cardinal 26.02. A tabela de
formatos é a do CHORD (#26), do próprio Modular.

Sem entrada polifônica (os cabos do Modular são mono: o acorde vem do
formato, não de notas tocadas), sem *latch* nem ordem de toque, sem swing
próprio (o swing vem do clock). A escuta do autor ainda não foi feita.
