# MARKOV — cadeia de Markov que aprende e improvisa

Implementação local de 9/11/2026, aprovada pelo autor a partir do estudo do
Cardinal ([`ESTUDO_cardinal_catalogo_e_licencas.md`](ESTUDO_cardinal_catalogo_e_licencas.md) §4b).
Fonte: [Markov.hpp](../src/dsp/Markov.hpp). Família DECISION; 10 HP.
Licença autoral: AGPL-3.0-or-later. **Fora do catálogo da seed** (entra pela
BIBLIOTECA), como LISTEN, PHRASE e ARP.

## Contrato

Primeira ordem sobre as 12 classes de altura: uma tabela 12×12 de transições.

**Entradas.** CLK (trig; livre → relógio interno em RATE), LRN.P (1 V/oct a
aprender), LRN.G (cada borda de subida lê LRN.P e conta a transição da nota
anterior para esta; a oitava da última nota ensinada vira a oitava de saída),
RST (esquece tudo e volta à tabela inicial).

**Saídas.** PTCH (1 V/oct = oitava aprendida + OCT + classe/12), GT (cada nota,
GATE × período), SURP (0–1: 1 − p/pmax da nota tocada).

**Aprender.** A linha da nota de origem é multiplicada por 0,5 + 0,5·MEM antes
de somar 1 na transição nova (MEM 1 = não esquece); teto de 64 por linha.
LEARN desligado congela a tabela.

**Sortear.** TEMP 0 → sempre a mais provável; TEMP 1 → as probabilidades
aprendidas; TEMP > 1 mistura com o uniforme sobre as transições já conhecidas
(p^(1/TEMP) + (TEMP − 1)·(1 − p^(1/TEMP))). Só notas já vistas a partir da
atual são candidatas.

**Tabela inicial.** Caminhada pela pentatônica {0, 2, 4, 7, 9}: vizinhos 1,
saltos de dois 0,4, ficar 0,2, mais longe 0,1; as outras classes voltam à
escala. Soa ao carregar.

**Desvios Rasgo.** TEMP, SURP como sinal, MEM (esquecimento).

## Testes

`tests/test_markov.cpp`: autônomo só na pentatônica e determinístico; ensinado
0 → 4 → 7 com TEMP 0, toca só esse ciclo e SURP = 0; TEMP 2 eleva a surpresa
média (0,03 → 0,53); LEARN desligado não aprende; robustez a NaN/∞.

## Proveniência e limites

Ideia pública: L. Hiller e L. Isaacson, *Illiac Suite* (1957); I. Xenakis,
*Analogique A* (1958–59). Conceito lido, sem cópia de código, no ZetaCarinae
Rosenchance e GuildensTurn (GPL-3.0+), dentro do Cardinal 26.02.

Primeira ordem só (não lembra frases, só pares); ritmo vem do clock, não é
aprendido.
