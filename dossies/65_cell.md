# CELL — autômato celular como sequenciador

Implementação local de 9/11/2026, aprovada pelo autor a partir do estudo do
Cardinal ([`ESTUDO_cardinal_catalogo_e_licencas.md`](ESTUDO_cardinal_catalogo_e_licencas.md) §4b).
Fonte: [Cell.hpp](../src/dsp/Cell.hpp). Família TIME; 10 HP.
Licença autoral: AGPL-3.0-or-later. **Fora do catálogo da seed** (entra pela
BIBLIOTECA), como LISTEN, PHRASE e ARP.

## Contrato

Uma linha circular de 8–24 células vivas ou mortas. A cada pulso (CLK, ou o
relógio interno em RATE) nasce a geração seguinte: cada célula olha para si e
para as duas vizinhas, e a regra elementar (0–255, Wolfram) decide se ela vive.

**Entradas.** CLK (trig; livre → relógio interno), RST (trig; sorteia uma linha
nova, semeada), RULE (CV somada à regra: 0–1 = +255).

**Saídas.** PTCH (1 V/oct: as três células depois da cabeça, lidas como binário
0–7, escolhem um grau da pentatônica maior {0, 2, 4, 7, 9, 12, 14, 16}; RANGE
0–2 soma uma oitava por célula viva em cabeça+4 e cabeça+5); GT (gate quando a
célula sob a cabeça está viva; duração GATE × período); DENS (fração de células
vivas, 0–1); EOS (pulso de 5 ms quando a linha morre ou para e renasce).

**Desvios Rasgo.** (a) MUT: a cada geração, chance semeada de inverter uma
célula; (b) linha morta (nenhuma viva) ou parada (igual à anterior) renasce com
um sorteio semeado (densidade 0,4) e dispara EOS; (c) RULE por CV.

**Tempo real.** process() não aloca; NaN/∞ viram neutros; xorshift semeado no
prepare() — mesma entrada, mesma saída. Mostrador "cell": as gerações rolando,
uma coluna por bloco.

## Testes

`tests/test_cell.cpp`: regra 90 a partir de uma célula (Sierpinski), regras 0 e
255, borda circular; autônomo e determinístico; regra 0 sem MUT renasce com EOS;
clock externo e RULE por CV; robustez a parâmetros fora da faixa e NaN/∞.

## Proveniência e limites

Regras elementares: S. Wolfram, "Statistical mechanics of cellular automata",
*Rev. Mod. Phys.* 55, 1983 (público). Conceito lido, sem cópia de código, no
voxglitch Glitch Sequencer (GPL-3.0+), dentro do Cardinal 26.02.

Uma dimensão só (sem o "jogo da vida" em grade); a leitura é sempre na
pentatônica maior — para outra escala, passar pelo QUANTIZER.
