# Dossiê — Módulo 38: Detector de nota, gate+pitch → evento (`NOTE-OUT`)

**Família:** MIX (observador/utilidade, junto do `SCOPE`)
**Estado:** **implementado — marco 3** (2026-09-05)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/NoteOut.hpp`, `tests/test_note_out.cpp`
**Origem:** contrato `NOTE` do `MUSICAL SCORE` —
`dossies/ESTUDO_seed_composicao_generativa.md §5`

## Estado da implementação (marco 3)

O adaptador que resolve o contrato `NOTE` **sem mudar a interface de
nenhum dos outros 37 módulos**. A ideia original ("precisaria que
`ENVELOPE`/`SEQUENCE`/etc. anunciassem seus próprios eventos de forma
genérica") foi a razão do item ficar em aberto por tanto tempo — a
solução adotada é diferente: `NOTE-OUT` é um OBSERVADOR que você cabeia
em GATE e PITCH onde quiser (o grafo digital deixa fan-out livre, não
precisa desconectar nada existente), e ele detecta borda de
nota-liga/nota-desliga sozinho.

- **`gate`**/**`pitch`** (entradas) — borda de subida do gate = nota
  liga (captura o `pitch` NAQUELE instante); borda de descida = nota
  desliga (fecha a nota, calcula a duração);
- **`velocity`**/**`accent`** (entradas opcionais) — amostradas no
  instante da nota-liga, não depois; sem conectar, `velocity`=1,0 e
  `accent`=falso;
- **`gate_thru`**/**`pitch_thru`** (saídas) — cópia exata das entradas.
  **Necessário pra rodar de verdade**: o motor só processa nós que
  chegam ao `sink` ativo (`setActiveOutput`, "órfãos não custam DSP por
  bloco") — um `NOTE-OUT` sem saída nenhuma nunca seria ancestral do
  sink e NUNCA teria `process()` chamado, mesmo com gate/pitch
  plugados. Por isso ele precisa ficar EM LINHA:
  `ALGO.gate → NOTE-OUT.gate → NOTE-OUT.gate_thru → ENVELOPE.gate` (ou
  qualquer destino que já exista) — mesma exigência de qualquer
  utilidade deste catálogo que precisa estar no caminho de verdade
  (`CONTROL`, `MULT`), confirmada por sonda dedicada (fora de linha:
  `process()` nunca roda; em linha: roda normalmente).
- **`takeCompletedNote(CompletedNote&)`** — método público (não porta),
  chamado de FORA de `process()` (o painel lê a cada bloco de áudio,
  nunca dentro do laço de amostra). Devolve `false` se não há nota nova
  desde a última leitura.

`CompletedNote{pitch, velocity, durationSeconds, accent}` — `pitch` é
**1V/oct CRU**: a conversão pra nome de nota/MIDI fica pra hora de
exportar (`MUSICAL SCORE`/MusicXML/MIDI, próxima camada, ainda não
feita), não presa aqui.

**v1 é MONOFÔNICO** — 1 `NOTE-OUT` capta 1 voz por vez; se uma
nota-liga chega enquanto outra ainda está ativa, a nota anterior nunca
fecha (fica pendurada até o próximo gate cair) — polifonia de verdade
(várias notas simultâneas, tipo o `CHORD`) fica de fora, documentado,
não escondida.

## Ponte com o RASGO Score

`apps/panel/panel_main.cpp` lê `takeCompletedNote()` de todo `NOTE-OUT`
em `shown`, a cada bloco de áudio, **só enquanto `[Ctrl+R]` está
gravando** — grava em `score.note(t, node, pitch, velocity, duration,
accent)`, com `t` = início da nota (o instante da conclusão menos a
duração), relativo à TOMADA (mesma unidade das mudanças de parâmetro já
gravadas). `apps/panel/ScoreRecorder.hpp` ganhou `RasgoEvent::Type::Note`
+ linha de texto própria (`note <nó> pitch=... velocity=... duration=...
accent=...`). `rasgo_modular_core` continua sem saber o que é
"partitura" — a ponte inteira vive em `panel_main.cpp`.

**Validação:** `tests/test_note_out.cpp` — **6 funções OK** (sem gate
nenhum, nunca há nota; detecta nota-liga/desliga com pitch e duração
corretos; velocity/accent amostrados no instante certo, não depois;
`gate_thru`/`pitch_thru` são cópia exata; determinismo; painel fecha).
Sonda dedicada confirma a exigência de estar em linha (fora do caminho
do sink, `process()` nunca roda). `tests/test_score_recorder.cpp`
ganhou 1 função pro evento `Note`. **45/45 CTest** Debug + Release. 5
renders de exemplo byte-idênticos (nenhum usa `NOTE-OUT`). Painel 8 HP,
0 sobreposições. Catálogo: família MIX, junto do `SCOPE`.

**Pendências:** exportação real pra `.mid`/`.musicxml` (só a captura
está feita); polifonia; não adicionado ao `PatchSeed.hpp`. Painel
compila, não testado visualmente.

**Não commitado.**
