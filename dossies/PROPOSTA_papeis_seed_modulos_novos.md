# Proposta — papéis dos módulos novos na seed, e patches mais complexos

Data: 21/11/2026. Estado: **implementado em 21/11/2026** ("ok para suas
recomendações"); as **cópias de vozes** do gene de tamanho (§3.1) e o ELASTIC
sem DRUM entraram em 23/11/2026 (§7). Pedido do autor:
"testei várias seeds do rasgo modular e nunca abriu os módulos novos"; "pode
propor"; e "com as implementações dos novos módulos, também atualizou como os
cabeamentos são feitos considerando patches mais complexos?".

## 1. O que acontece hoje (medido em 400 seeds, 21/11)

- Os módulos novos entram pela **passada genérica**: a seed os liga como
  "mais um", sem saber o que cada um é.
- 54 % das seeds trazem algum dos 12 de 20/11 — quase sempre um **modulador**
  (SEASON 19 %, SWARM 18 %, ECLIPSE 16 %, REFRACT 15 %, PRESSURE 14 %).
- Só **14 %** trazem uma fonte ou um processador de som novo: GENDYN 2 %,
  ELASTIC 2 %, LENS 1 %, TAPE 5 %, RUNGLER 5,5 %. Por isso eles "não abrem".
- Papéis dedicados existem para seis módulos de antes (INTÉRPRETE, RELAÇÃO,
  POLÍMETRO, ÍMÃS, EPICICLO, CORPO — `PatchSeed.hpp`), sorteados na ordem da
  seed, **no máximo dois por seed**, cada um com a sua chance.
- A forma do patch (quantas vozes, que funções) não mudou nesta leva.

## 2. Papéis propostos (um por módulo, no mesmo molde)

Cada papel tem condição, chance e o que liga. Os números são o ponto de
partida; a régua de qualidade das seeds vigia cada um.

| Papel | Módulo | O que a seed faz | Chance |
|---|---|---|---|
| **VOZ ESTOCÁSTICA** | GENDYN | entra como a voz de textura (canal 3 ou 4), FREQ na tônica da seed, BAND estreito; o ENVELOPE da textura abre o VCA | 12 % |
| **CORAÇÃO** | RUNGLER | os degraus do registrador modulam o cutoff do filtro da voz principal; ou vira a fonte de uma camada percussiva | 12 % |
| **CORPO ELÁSTICO** | ELASTIC | ressonador depois do DRUM ou de um ruído com gate: o golpe vira um corpo inarmônico | 10 % |
| **LENTE** | LENS | no caminho do retorno de espaço (no lugar do SPACE/HALL), DEPTH baixa, BLUR lento pelo DRIFT | 8 % |
| **FITA** | TAPE | na voz principal ou antes do MASTER, WEAR baixo (o caráter, não o defeito) | 12 % |
| **GÁS** | PRESSURE | os choques no pistão viram os gates de uma camada percussiva (no lugar de um TRIGSEQ) | 10 % |
| **RAIO** | REFRACT | o ângulo modula uma cor (formante, wavetable); a reflexão total vira acento | 8 % |
| **FRANJAS** | INTERFERENCE | as franjas claras disparam uma camada; o moiré faz o polirritmo | 8 % |
| **ECOLOGIA** | PREDATOR | presa e predador em dois cutoffs (ou pan de duas vozes); capturas disparam notas | 8 % |
| **CARDUME** | SWARM | três a seis relógios do cardume tocam vozes copiadas: heterofonia ↔ uníssono | 8 % |
| **ECLIPSE** | ECLIPSE | o evento raro dispara uma mudança: o fill do TRIGSEQ ou o NEW da VOLTA | 6 % |
| **ESTAÇÃO** | SEASON | forma longa: o modo da estação no SCALE do QUANTIZER, o andamento pelo RATE do CLOCK | 6 % |
| **VOLTA** | VOLTA | no lugar do TURING na linha melódica, com MUT por um FUNCTION lento | 10 % |
| **DUAS MÃOS** | SUM | dois moduladores somados num mesmo parâmetro (DRIFT + FUNCTION no cutoff) | 10 % |

- **POLY-IN, FORM e TEMPO** ficam fora (o primeiro é mudo sem teclado; FORM e
  TEMPO são a decisão maior de a seed ganhar começo, meio e fim — §3).
- **Limite:** hoje são no máximo dois papéis por seed. Com 20 papéis, sugiro
  **três**, com a régua conferindo que o patch não fica denso demais
  (memória do autor: "qualidade primeiro… sem densidade por si só").

## 3. Patches mais complexos (o caminho)

Hoje a seed monta uma espinha (relógio → sequência → quantizador → voz →
envelope → mixer) e enfeita com papéis. Para patches maiores, sem perder a
qualidade:

1. **Tamanho como gene da seed** — *camerístico* (1–2 vozes), *médio* (3) e
   *orquestral* (4 vozes, com cópias de módulos: dois OSC, dois ENVELOPE…).
   As cópias já existem (instâncias); falta a seed pedi-las com papel (o
   passo 3 das instâncias, adiado em 11/11).
2. **Vozes com função**: baixo, melodia, contraponto, textura — cada uma com
   o seu registro e o seu ritmo, como no Trioio e no Batruc (a "condução de
   vozes por papel").
3. **Modulação cruzada pelo SUM**: dois movimentos num mesmo parâmetro, a
   base dos patches complexos de modular.
4. **Forma**: numa seed *orquestral*, o FORM e o TEMPO como espinha da forma
   (todas as vozes passam pelo FORM; o CLOCK pelo TEMPO). Essa é a decisão
   maior da proposta de forma: a seed passaria a ter começo, meio e fim.

## 4. Como implementar e medir

- Um papel por vez, no molde dos seis atuais; cada um com teste (o papel
  liga o que promete) e a régua de qualidade verde.
- Medir de novo a presença: a meta é cada módulo novo aparecer **ligado ao
  som** em 8–15 % das seeds.
- As seeds mudam (como a cada módulo novo); fica no CHANGELOG.

## 5. Decisões do autor

1. Os papéis e as chances da tabela (ou quais primeiro).
2. Três papéis por seed em vez de dois.
3. O tamanho como gene (camerístico / médio / orquestral).
4. FORM e TEMPO nas seeds orquestrais (decisão maior).

## 6. Implementado (21/11/2026)

- `PatchSeed.hpp`: o **tamanho** da seed (corrente 95): camerístico 30 % (até
  1 papel), médio 50 % (2), orquestral 20 % (3 e a forma). Os 14 papéis da
  tabela, na corrente 120…, depois dos papéis de antes e até o teto; cada um
  volta "como era" se um cabo falha.
- **Orquestral:** as vozes do MIXER passam pelo FORM (DEPTH 6–9 dB, LEN 3–7
  min, LOOP) e o CLOCK pelo TEMPO (RATE), com TENS, SECT e END ligados.
- Medido em 400 seeds (antes → depois, ligados ao som): GENDYN 2 → 6,5 %,
  RUNGLER 5,5 → 8 %, LENS 1 → 5,5 %, TAPE 5 → 9 %, ELASTIC 2 → 3 % (só com
  DRUM no MIXER), VOLTA 10,5 %, SUM 8,5 %, FORM/TEMPO 17 %; seeds com uma
  fonte ou processador novo: 14 → 29 %. Nenhuma seed caiu no recurso de
  emergência (seed 1). Régua de qualidade verde (suíte 135/135).
- **As seeds mudam** (CHANGELOG).
- Falta: as cópias de vozes (orquestral com 4 vozes de verdade), e subir o
  ELASTIC (um papel que dispense o DRUM).

## 7. Cópias de vozes e ELASTIC sem DRUM (23/11/2026)

- **Medido antes:** das 69 seeds orquestrais (em 400), 21 tocavam com uma
  voz só no MIXER e 12 com duas — o "orquestral" não aparecia.
- **Cópias de vozes** (`PatchSeed.hpp`, corrente 70–93): nas orquestrais,
  cada canal vazio do MIXER recebe uma STRING ou um MATTER ainda sem cabos
  no rack (as cópias de fábrica), pinçado por ACC, EUC ou CLK do CLOCK e com
  o 1V/O no PITCH do QUANTIZER (a mesma harmonia da voz principal). A
  primeira cópia fica uma oitava abaixo da voz (o baixo, −12 a −8 dB), as
  outras uma acima (−16 a −12 dB). Depois passam pelo FORM como as outras.
  Resultado: orquestrais com 4 vozes 13 → 56 de 69; as que ficam com menos
  não têm QUANTIZER na espinha ou têm voz sem altura (ruído).
- **ELASTIC sem DRUM:** a corda de massas entra como voz pinçada pelo EUC ou
  pelo ACC do CLOCK, num canal livre (disputa o canal com o GENDYN). Ligado
  ao som: 3 → 6,75 %; fontes/processadores novos 29 → 31,5 %.
- As seeds camerísticas e médias ficaram idênticas (mesma distribuição de
  canais). Ainda falta: vozes com função rítmica própria (o contraponto do
  Trioio/Batruc, §3.2) — hoje as cópias dividem a altura da voz principal e
  variam no registro e no ritmo do pinçar.
