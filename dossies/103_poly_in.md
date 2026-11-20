# 103 — POLY-IN — o MIDI polifônico em até 4 vozes

Data: 20/11/2026. Estado: **implementação local**, aguardando a conferência do autor.

Pedido do autor: "também havia comentado sobre as polifonias, como pretende
resolver isso ?" → a proposta de um adaptador que reparte o MIDI em vozes,
com as cópias de módulo (até 4) fazendo o resto; "ok" no mesmo dia. Sem
cabo polifônico no núcleo: cada voz é um cabo comum. O passo 2 (cabos de
várias vozes, como no VCV/Cardinal) só se este não bastar.

Código: `src/dsp/PolyIn.hpp`. Família SOURCE (ao lado do SIGNAL-IN).

## Desenho

- Saídas por voz: `P1–P4` (1 V/oct, nota 60 = 0 V, + bend), `G1–G4`
  (gate, rampa de 1 ms), `V1–V4` (velocidade 0–1).
- `VOZES` (1–4), `MODE` (ROTA / MESMA / ALTURA), `BEND` (0–24 st), `ON`.
- **ROTA**: a próxima voz livre, em rodízio; **MESMA**: a nota repetida
  volta à voz em que tocou, senão a livre há mais tempo; **ALTURA**: as
  vozes em ordem de altura (voz 1 = a mais grave que soa; notas além das
  vozes, as mais agudas, ficam de fora). Sem voz livre: rouba a mais antiga.
- Voz roubada ou reaproveitada: o gate fecha 2 ms antes da nota nova (o
  ENVELOPE redispara). CC 64 (pedal) segura; CC 123 solta tudo.
- MIDI pelo mesmo anel do SIGNAL-IN; o app entrega a ambos. Mensagens de
  sistema (clock) ficam para o SIGNAL-IN.
- **Só MIDI:** com o POLY-IN sozinho, o app não abre a entrada de áudio (o
  macOS não pede o microfone). No plugin, o MIDI vem da DAW.

## Seed e VARIA

No rack da seed (regra do autor de 9/11), mas **as saídas ficam fora do
sorteio dos cabos** (`SeedGrammar.hpp`): sem teclado elas são mudas. O ON
não é sorteado nem herdado; o VARIA não mexe nele (aparelho de quem toca).

Testes (`test_poly_in`): acorde em ROTA, rodízio, roubo da mais antiga,
redisparo, MESMA, ALTURA (o baixo troca quando a nota grave sai), pedal,
bend, ON desligado.

Referências: a alocação de vozes dos sintetizadores polifônicos; o MIDI-CV
polifônico do VCV Rack (GPL) — conceito, nenhum código.
