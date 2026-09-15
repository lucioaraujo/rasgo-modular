# License status — Rasgo Modular (front-end JUCE)

**Status atual: código próprio AGPL-3.0-or-later; JUCE 9.0.0 usado sob
AGPL-3.0-only. Nenhuma licença comercial do JUCE é necessária.**

## Por que este arquivo existe

Convenção da família RASGO: todo projeto que compila contra o JUCE registra
por escrito sob qual opção de licença o faz, pra que a decisão não fique
implícita no `CMakeLists.txt`. O precedente é
[`NAVALHA2_JUCE/docs/LICENSE_STATUS.md`](../../../NAVALHA2_JUCE/docs/LICENSE_STATUS.md).

## As duas licenças em jogo

- **O código do Rasgo Modular** (motor em `src/`, painel de teste em
  `apps/panel/`, este front-end em `apps/juce/`) está sob
  **AGPL-3.0-or-later** — registrado no `LICENSE` da raiz do projeto e no
  ingresso arquivístico `ARQ-RSM-001` desde o marco 1 (2026-09-01).
- **O JUCE** é distribuído pelo fornecedor sob duas opções: uma licença
  comercial paga, ou a **AGPL-3.0-only**. Este projeto usa a segunda.

## Por que a combinação é limpa aqui

Este caso é mais simples que o do Navalha 2. Lá, o código próprio é
GPL-3.0-or-later e a compatibilidade depende da Seção 13 da GPLv3 (que
autoriza explicitamente combinar GPLv3 com AGPLv3). Aqui, **o código
próprio já é AGPL-3.0-or-later** — a mesma família de licença do JUCE. Não
há combinação de licenças distintas a justificar: o "or later" cobre a
AGPL-3.0, e o resultado distribuído é coerentemente AGPL.

## Efeito prático da AGPLv3

A obrigação característica da AGPLv3 é a cláusula de interação por rede
(Seção 13): se uma versão modificada oferecer interação remota a usuários
por uma rede, o código-fonte correspondente precisa ser oferecido a esses
usuários.

O front-end JUCE do Rasgo Modular é um aplicativo **desktop local**, sem
componente de servidor. Hoje essa cláusula não é acionada pelo uso normal.
Fica registrada porque o `§36.7` prevê um segundo front-end **web/WASM** —
se ele for hospedado como serviço, a obrigação passa a valer e a oferta de
fonte precisa acompanhar o deploy.

## O que checar antes de distribuir

- O `LICENSE` da raiz (AGPLv3) vai junto no pacote — é o que o
  `CPACK_RESOURCE_FILE_LICENSE` aponta.
- O JUCE **não** é versionado neste repositório (nem submodule, nem
  FetchContent): é um checkout local que o desenvolvedor/CI aponta via
  `-DRASGO_MODULAR_JUCE_PATH=...`. Quem redistribuir o binário está
  distribuindo uma obra combinada e deve oferecer também a fonte do JUCE
  na versão usada.
- A versão exata do JUCE usada em cada release deve ser registrada na
  própria release (a CI faz checkout do `master` do JUCE; para uma release
  reprodutível, fixar um commit/tag).
