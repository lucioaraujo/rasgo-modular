#pragma once

// ============================================================================
// AUDIO-IN → SIGNAL-IN (Módulo 35 → 49)
// ============================================================================
//
// O `AUDIO-IN` cresceu pra o `SIGNAL-IN` (áudio + MIDI + CV num adaptador
// só — decisão do autor 2026-09-06, `PESQUISA_MODULOS.md §2.4` #49).
// Este header vira só um alias pra não quebrar `#include "dsp/AudioIn.hpp"`
// nem o nome `AudioIn` no painel. O tipo de módulo é "SIGNAL-IN";
// `makeModule("AUDIO-IN")` ainda funciona (alias no `ModuleCatalog`), e
// re-salvar um `.rmp` antigo migra o tipo.

#include "dsp/SignalIn.hpp"

namespace rasgo::modular {
using AudioIn = SignalIn;
}
