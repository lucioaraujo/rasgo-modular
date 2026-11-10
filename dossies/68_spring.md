# SPRING — reverb de mola

Implementação local de 9/11/2026, aprovada pelo autor a partir do estudo do
Cardinal ([`ESTUDO_cardinal_catalogo_e_licencas.md`](ESTUDO_cardinal_catalogo_e_licencas.md) §4b).
Fonte: [Spring.hpp](../src/dsp/Spring.hpp). Família SPACE; 10 HP.
Licença autoral: AGPL-3.0-or-later. **Fora do catálogo da seed** (entra pela
BIBLIOTECA), como LISTEN, PHRASE e ARP.

## Contrato

Duas molas (laços de comprimento 1 : 1,137). Em cada uma: linha de atraso
(25–75 ms por TENS, leitura com interpolação linear e tensão suavizada em
~50 ms) → cascata de 12 all-pass esticados H(z) = (a + z^−3)/(1 + a·z^−3), com
a = 0,35 + 0,4·DRIP (a dispersão: graves depois dos agudos, o chirp) →
passa-baixa de um polo (6 kHz → ~1,2 kHz por DAMP) → anti-DC → realimentação
DECAY (0–0,97; padrão 0,85, cauda de ~2 s).

**Entradas.** IN (áudio mono/estéreo, somado), KICK (trig), TENS (CV, ±0,5).

**Saídas.** OUT (seco × (1 − MIX) + molhado × MIX; em estéreo, uma mola por
canal), WET (só a mola). Limitado a ±4; NaN/∞ viram zero.

**Desvios Rasgo.** (a) KICK: 30 ms de ruído semeado, decaindo, direto nas molas
— o "crash"; (b) TENS por CV (a mola geme, sem clique).

As linhas são alocadas no prepare(); process() não aloca.

## Testes

`tests/test_spring.cpp`: cauda decai (energia 0,17 → 9·10⁻⁵ → 7·10⁻⁹ em 0,3 /
1–1,3 / 2,5–3 s); DECAY maior → cauda mais longa; nada antes de 20 ms com TENS 0;
MIX 0 = entrada exata; KICK sem entrada soa; sem entrada e sem KICK, silêncio;
TENS por CV e parâmetros extremos finitos e limitados.

## Proveniência e limites

Formulação pública: V. Välimäki, J. Parker, J. S. Abel, "Parametric Spring
Reverberation Effect", *JAES* 58(7/8), 2010. Conceito lido, sem cópia de código,
no Autinn Coil (GPL-3.0+), dentro do Cardinal 26.02.

Modelo simplificado (sem o ramo de alta frequência do artigo nem o modelo de
transdutor); a escuta do autor ainda não foi feita.

## Leitura fracionária por Lagrange cúbico (10/11/2026, correção técnica aprovada pelo autor)

A linha de atraso era lida com interpolação **linear**, que é um passa-baixa
dependente da fração: no meio do caminho entre duas amostras, 8 kHz a 48 kHz
sai ~1,25 dB mais baixo, e quando o atraso anda o brilho oscila junto; dentro
do laço com realimentação a perda se acumula a cada volta. Agora a leitura é
Lagrange de 3ª ordem (`src/dsp/DelayRead.hpp`; Laakso, Välimäki, Karjalainen
& Laine, "Splitting the Unit Delay", 1996), com recuo para a linear abaixo
de 3 amostras de atraso. Teste em `test_spring` (`testFractionalReadKeepsTheTreble`):
linear < −1 dB, Lagrange entre −0,35 e +0,1 dB. Muda levemente o som: caudas
um pouco mais brilhantes.
