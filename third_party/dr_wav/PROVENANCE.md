# dr_wav — proveniência

**Arquivo:** `dr_wav.h` (single-header, ~9200 linhas)
**Projeto:** dr_libs — https://github.com/mackron/dr_libs
**Autor:** David Reid (mackron@gmail.com)
**Versão:** v0.14.6 (em desenvolvimento à data)
**Origem:** `https://raw.githubusercontent.com/mackron/dr_libs/master/dr_wav.h`
**Commit do repositório na captura:** `dfe8377631000664666519fdb83da193fd8037f4`
(2026-08-31)
**Baixado:** 2026-09-06

## Licença

**Escolha do usuário: domínio público (Unlicense) OU MIT-0.**
Ambas compatíveis com AGPLv3-or-later do RASGO Modular
(`RASGO_MODULAR.md §29` / `PESQUISA_MODULOS.md §1`). O texto integral das
duas licenças está no fim do próprio `dr_wav.h` (ALTERNATIVE 1 / 2).
Sem CC-NC/ND. Sem cabeçalho SPDX adicionado.

## Alterações locais

**Nenhuma.** O arquivo é usado verbatim. A macro `DR_WAV_IMPLEMENTATION`
é definida em **um** `.cpp` (`src/io/AudioFile.cpp` — quando existir; hoje
`src/io/AudioFile.hpp` é header-only e define a implementação num bloco
guardado). O resto do projeto só faz `#include`.

## Por que este e não outro

Decisão registrada em `dossies/ESTUDO_audio_sampling.md §3`: single-header,
zero dependência de sistema, mesma pegada do nosso `WavWriter`
(round-trip garantido — `writeWav16` gera PCM/float que o `dr_wav` lê).
`libsndfile` (LGPL + link dinâmico) e FFmpeg descartados.

## Uso

Só a camada `src/io/` (fora do `rasgo_modular_core`, que continua
sem dependência). O `SAMPLER` (Módulo 48) recebe um `std::vector<float>`
— não conhece `dr_wav`; carregar arquivo é gesto de UI.
