# Dossiê — Módulo 7: Memória granular (`MEMORY`)

**Família:** MEMORY
**Estado:** **implementado — marco 1** (2026-09-02)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Memory.hpp`, `tests/test_memory.cpp`

## Estado da implementação (marco 1)

Feito: buffer circular de 3 s grava o que entra; `freeze` (parâmetro ou
gate) para a gravação → o buffer vira textura fixa; um pool de 16 grãos
janelados (Hann) relê o material.

- **grão:** janela curta (`grain` 5–500 ms) com envelope Hann,
  interpolação linear na leitura, velocidade `speed = 2^(pitch/12)`
  (`pitch` −24..+24 st + `pitch_mod` 1 V/oct);
- **nuvem:** grãos novos agendados a `density` Hz; pool cheio → grão
  descartado (comportamento de nuvem, não fila);
- **posição:** `position` (0–1) lê de "agora" para "3 s atrás";
  `spray` randomiza a posição por grão (xorshift semeado);
- **feedback:** `feedback` (0–0,95) realimenta a saída na gravação
  (quando não congelado); clamp em ±1,5 antes de escrever;
- **blend:** `blend` (0–1) seco/molhado; `blend = 0` → saída idêntica
  à entrada.

Determinístico (rng semeado em `prepare()`). Buffer alocado em
`prepare()`; `process()` não aloca.

**Testes (8/8 alvos, Debug + Release):** textura granular do que entrou
(RMS > 0,02 depois de encher); **freeze segura a textura mesmo com a
entrada em silêncio** (RMS pós-freeze > 40% do pré); `blend = 0` →
saída byte-a-byte igual à entrada; `pitch` de −12 a +19 st sem NaN nem
estouro; dois renders byte-idênticos; 3000 blocos estéreo com
feedback 0,6 + freeze intermitente sem NaN; integração no grafo
(`FUNCTION → MEMORY` com pitch +7); painel fecha (14 HP).

**Pendências (candidatos, não controles fictícios):** captura por onset
do próprio sinal (arbhar — autoescuta dispara a gravação); `texture`
(forma de janela: Hann ↔ retangular ↔ Tukey); modo estéreo real (grãos
espalhados no campo); reverso por grão; `size` do buffer como parâmetro
(hoje fixo em 3 s); interpolação cúbica na leitura; anti-click no
`freeze` (crossfade de 5 ms); limitar CPU do pool com `density` alto +
`grain` longo (16 grãos de 500 ms = 16 leituras/amostra, medir).

---

## 1. Problema musical e papel no fluxo

A cicatriz do `Cable` (Atlas §37) já dizia: **romper não é apagar, é
segurar o último bloco**. `MEMORY` é essa ideia como módulo pleno — o
patch grava a si mesmo e pode voltar a tocar o que gravou, transposto,
espalhado, congelado. Numa lógica generativa isso fecha um ciclo: o que
o sistema produziu vira material do que ele vai produzir. É a diferença
entre um instrumento que só avança e um que se lembra.

Papel: recebe a voz articulada (`ENVELOPE.out`) ou o `FILTER.all`;
`freeze` disparado por um gate do `CLOCK`/`DECISION` cria seções
suspensas; a saída granular volta ao `FILTER` ou direto à saída. Com
`feedback` alto e `freeze` desligado, é um loop que se degrada.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Mutable Clouds** (`pichenettes/eurorack`, STM32F4) | buffer + textura granular + `freeze`; os eixos position / size / density / texture / pitch / blend / feedback como um espaço contínuo | MIT — estudo do comportamento, sem código |
| **Instruō arbhar** (hardware) | captura por onset do próprio mix; autoescuta como fonte de material; grão como unidade expressiva | hardware, estudo de comportamento |
| **Curtis Roads**, *Microsound* (2001) | grão = janela curta com envelope; nuvem = muitos grãos assíncronos; densidade e dispersão como parâmetros de textura | teoria pública |
| **Janela de Hann** (Blackman-Harris et al.) | envelope de grão sem clique nas bordas, lóbulo lateral aceitável | matemática pública |
| **`Cable` scar/freeze** (Atlas §37, já no Rasgo) | o precedente interno: reter o último material e repetir decaindo | conceito próprio |

## 3. Modelo — matemática, estados, extremos

**Buffer.** `buffer_[bufLen]`, `bufLen = 3·sr`, escrita circular em
`writePos_`. Gravação só quando não congelado:
`buffer_[writePos_] = clamp(in + feedback·lastOut_, ±1,5)`.

**Agendamento de grão.** `grainClock_ += density/sr`; ao passar de 1,0,
`spawnGrain()`.

**`spawnGrain`.** Acha um slot livre no pool (senão descarta). Ponto de
partida, lendo pra trás do cabeçote de escrita:
```
back  = (0,03 + position·0,9 + spray·rand01·0,4) · bufLen
start = wrap(writePos_ − back)
speed = 2^(pitchSemis/12)
phaseInc = 1 / (grain·sr)
```

**Leitura de grão** (por amostra, por grão ativo):
```
window = 0,5 − 0,5·cos(2π·phase)
sample = lerp(buffer_[⌊readPos⌋], buffer_[⌊readPos⌋+1])
wet   += sample · window
readPos += speed   (wrap em bufLen)
phase  += phaseInc  →  phase ≥ 1  ⇒  grão inativo
```
`wet · 0,6` (compensação de sobreposição). `out = blend·wet +
(1−blend)·dry`. `lastOut_ = out`.

**Extremos.** `density` mínimo (0,1 Hz) → um grão a cada 10 s, pool quase
sempre vazio. `density` máximo (120 Hz) + `grain` máximo (0,5 s) → ~60
grãos sobrepostos pedidos, mas o pool tem 16 → os excedentes são
descartados (limite duro, sem alocar). `pitch` = +24 → `speed` = 4,
`readPos` anda 4 amostras/amostra, ainda dentro do buffer. `feedback`
teto 0,95 + clamp ±1,5 → não diverge. `freeze` com buffer ainda não
cheio no começo → grãos leem zeros (silêncio, correto). Reset →
`buffer_` zerado, `writePos_ = 0`, pool limpo, rng resemeado.

## 4. Três modos obrigatórios

- **Autônoma:** com `feedback` alto e `blend` alto, um curto material
  inicial vira um loop que se transforma sozinho (os grãos re-espalham o
  que o feedback reescreve). `freeze` fixa um instante como drone.
- **Performance:** `position`, `pitch` e `freeze` são os gestos — varrer
  `position` "rebobina" audivelmente; `freeze` congela ao vivo;
  `spray`/`density` abrem de "eco" a "nuvem".
- **Híbrida:** `freeze_gate` do `CLOCK` cria seções suspensas no tempo do
  patch; `position_mod` de um LFO faz a leitura vaguear; `pitch_mod` de
  um `DECISION`/`TURING` transpõe a memória em degraus.

## 5. Portas, parâmetros, limites

**Entradas:** `in` (Audio), `position_mod` (Control), `pitch_mod`
(Control, 1 V/oct), `freeze_gate` (Control, nível segura o freeze).
**Saídas:** `out` (Audio).
**Parâmetros:**
| id | faixa | default | o que faz |
|---|---|---|---|
| `grain` | 0,005–0,5 s | 0,08 | comprimento do grão |
| `density` | 0,1–120 Hz | 20 | grãos por segundo |
| `position` | 0–1 | 0,2 | de "agora" a "3 s atrás" |
| `spray` | 0–1 | 0,1 | randomização da posição por grão |
| `pitch` | −24..+24 st | 0 | transposição da leitura |
| `feedback` | 0–0,95 | 0 | realimentação na gravação |
| `blend` | 0–1 | 1 | seco/molhado |
| `freeze` | 0/1 | 0 | para a gravação |

**Limites:** saída limitada pelo material e pelo `·0,6`; pool fixo de 16
grãos (custo máximo previsível). Buffer 3·sr floats alocado em
`prepare()`. `process()` sem alocação.

## 6. Alternativas descartadas

- **Buffer de duração variável em runtime:** realocaria; 3 s fixos
  cobrem eco→textura→drone. `size` como parâmetro (dentro do buffer
  fixo, como "janela de captura") é 2ª camada.
- **FFT / phase vocoder pra time-stretch:** outra família (espectral,
  PERCEPTION); o granular assíncrono dá o mesmo resultado musical com
  muito menos código e latência.
- **Fila de grãos (voice-stealing ordenado):** o descarte simples é o
  comportamento de nuvem certo (Clouds faz igual); roubar o grão mais
  antigo introduziria cliques.
- **Interpolação cúbica já no marco 1:** linear alia um pouco em
  `pitch` alto; cúbica é 2ª camada (mesma lógica do oversampling do
  `FILTER`).

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** com entrada senoidal e `pitch = +12`, o pico espectral da
saída sobe uma oitava (±3%); `freeze` mantém RMS estável com entrada
zerada; `blend = 0` byte-a-byte igual à entrada; `feedback = 0,9` não
diverge em 60 s; dois renders byte-idênticos; sem NaN em varredura de
`pitch`/`spray`; sem alocação em `process()` (teste).

**Escuta:** varrer `position` soa como rebobinar uma fita ou como
glitch? o `freeze` congela "limpo" ou com clique? `density` subindo vai
de eco a nuvem de forma contínua? `feedback` alto + `spray` soa vivo ou
só embola? `ENVELOPE → MEMORY(freeze) → saída` cria uma seção suspensa
convincente?

## 8. Integração e painel

Classe `Memory` (`type()` = `"MEMORY"`), 4 entradas, 1 saída, 8
parâmetros. `panel()` próprio (14 HP: display do buffer +
GRAIN/DENS/POS/SPRAY, PITCH/FBK/BLEND/FREEZE, jacks). Testado isolado
(captura, freeze, blend, pitch, determinismo) antes do patch. Entra na
peça longa como a camada de memória: congela seções, transpõe o
material já tocado, fecha o ciclo generativo.
