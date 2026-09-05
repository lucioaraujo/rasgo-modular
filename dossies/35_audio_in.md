# Dossiê — Módulo 35: Entrada de áudio ao vivo (`AUDIO-IN`)

**Família:** SOURCE
**Estado:** **implementado — marco 3** (2026-09-04)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/AudioIn.hpp`, `apps/panel/AlsaSource.hpp`,
`tests/test_audio_in.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.2` — pedido direto do autor: "como
podemos conectar o antitotem no rasgo modular? [...] isso qualquer fonte
externa"

## Estado da implementação (marco 3)

O adaptador que deixa o instrumento OUVIR. Identidade do RASGO Modular:
"MIDI/audio/instrument coupling são nós adaptadores OPCIONAIS, nunca
dependência" (`RASGO_MODULAR.md §36.8`) — o painel soa sozinho ao
carregar, com ou sem `AUDIO-IN`; nenhum seed aleatório inclui este módulo
(`PatchSeed.hpp` não itera o catálogo às cegas — é curado à mão, e
`AUDIO-IN` nunca entra nessa lista de propósito).

**Por que dois arquivos:** `rasgo_modular_core` é zero-dependência — não
sabe o que é ALSA. Mesma separação já usada pra saída
(`AlsaSink.hpp`/`Master.hpp`):

- **`src/dsp/AudioIn.hpp`** (motor, zero-dep): o nó `Signal`. Um anel
  circular SPSC (produtor único/consumidor único) sem alocação em
  `process()`. `pushSamples(interleavedLR, frames)` — lado PRODUTOR,
  chamado de fora do thread de áudio do grafo, nunca bloqueia (se o anel
  está cheio, sobrescreve o mais antigo). `process()` — lado CONSUMIDOR,
  lê do anel; em underrun (captura mais devagar que o consumo) preenche
  silêncio em vez de travar ou ler lixo; em estouro (produtor sobrescreveu
  dado ainda não lido), resincroniza pra janela mais recente em vez de ler
  uma volta antiga do anel. **Sem nenhuma captura alimentando (painel sem
  `AUDIO-IN` patcheado, ou os testes/exemplos, que nunca chamam
  `pushSamples`), a saída fica exatamente em silêncio — determinístico,
  testável sem hardware nenhum.**
- **`apps/panel/AlsaSource.hpp`** (painel, ALSA): contraparte de captura
  do `AlsaSink.hpp` — mesmo padrão de `snd_pcm_hw_params`, mesmo fôlego
  de 8 períodos. `read()` bloqueante; ao contrário da escrita (que
  precisou de um relógio de parede manual nesta camada PipeWire/ALSA — a
  fila de saída aceita tudo sem reagir), a leitura pacia sozinha pela
  chegada real de amostras.

**Fiação no painel** (`panel_main.cpp`):

- `syncAudioIn()` — chamado em todo ponto que já chama `buildMods()`
  (seed novo, carregar sessão/banco, adicionar/remover módulo): conta
  quantos nós `AUDIO-IN` existem no patch AGORA; abre a captura só se
  houver pelo menos 1 e ainda não estiver aberta; fecha e devolve o
  dispositivo assim que o último `AUDIO-IN` sai do patch;
- **thread PRÓPRIA**, separada da `audio` (que já toca o grafo) — de
  propósito: a pacing da captura não deve arriscar a temporização já
  delicada da reprodução (os comentários de `pace()` no `AlsaSink`
  documentam o trabalho que aquilo já deu). O único acoplamento entre as
  duas é `AudioIn::pushSamples()`, lock-free por dentro;
- a thread de captura só usa `gmx` (`try_lock`) pra ENUMERAR os nós com
  segurança — se a UI está editando o patch, pula aquele período (perde
  um pedaço de captura) em vez de esperar; nunca trava a UI.

**Como conectar de verdade:** o sistema já roda PipeWire — abra o
`AUDIO-IN` no patch (a captura ALSA "default" aparece como um nó de
entrada no grafo do PipeWire) e ligue a saída de qualquer outro app
(`pw-link`, ou um patchbay gráfico tipo `qpwgraud`/`helvum`, nenhum
instalado neste sistema ainda) na entrada correspondente. Serve pra
QUALQUER fonte externa — ANTITOTEM foi só o gatilho da pergunta.

**Validação:** `tests/test_audio_in.cpp` — **6 funções OK** (silêncio sem
alimentação; ida-e-volta exata com `gain=1`; `gain` aplicado; underrun
preenche silêncio sem travar/sujar; estouro por acúmulo resincroniza pra
janela mais recente, não lê dado sobrescrito; determinismo). **42/42
CTest** Debug + Release. 5 renders de exemplo byte-idênticos (não usam
`AUDIO-IN`, não usam nada de `apps/panel/`). Painel compila; **não
testado com hardware/PipeWire de verdade** — só a lógica do anel foi
verificada (single-threaded, nos testes); a captura ALSA em si e a
fiação da thread não podem ser testadas no CTest (dependem de
dispositivo real) nem por mim (não rodo o painel gráfico — combinado do
projeto).

**Pendências:** seleção de dispositivo (hoje só `"default"` — um
`RASGO_AUDIO_IN_DEVICE` env var seria o próximo passo, mesmo padrão do
`RASGO_SEED`); medidor de nível no painel (`Display` do widget hoje é só
rótulo, sem desenho de VU); mono-sum opcional como saída extra.

**Não commitado.**
