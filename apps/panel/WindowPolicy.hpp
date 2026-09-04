#pragma once

// Política de janela do painel de teste — geometria pura, sem X11 nem
// JUCE (mesma ideia de `AQUORBIUM/src/app/WindowLayoutPolicy.h`). Separa
// o CANVAS DE REFERÊNCIA do TAMANHO OBRIGATÓRIO da janela
// (`RASGO_DOCUMENTATION/design/INTERFACES_E_LAYOUTS.md §5.1`).
//
// Aqui a versão mínima: abrir em ~88% da área útil de UM monitor (o
// primário), sem passar do canvas de referência do conteúdo, centrado
// nesse monitor. Persistência de bounds e reflow semântico ficam pro
// front-end de produção.

#include <algorithm>

namespace rasgo::panel {

struct MonitorRect {
    int x = 0, y = 0, w = 1366, h = 768;
};

struct WindowBounds {
    int x = 0, y = 0, w = 1024, h = 640;
};

// `contentW/H` = canvas de referência do conteúdo (o que o layout quer).
// `usableFrac` ~ 0,85..0,90.
inline WindowBounds firstOpen(const MonitorRect& mon, const int contentW,
                              const int contentH, const float usableFrac = 0.88f) {
    const int maxW = static_cast<int>(static_cast<float>(mon.w) * usableFrac);
    const int maxH = static_cast<int>(static_cast<float>(mon.h) * usableFrac);
    WindowBounds b;
    b.w = std::max(640, std::min(contentW, maxW));
    b.h = std::max(400, std::min(contentH, maxH));
    b.x = mon.x + (mon.w - b.w) / 2;
    b.y = mon.y + (mon.h - b.h) / 2;
    return b;
}

// Reencaixa bounds restaurados no monitor mais próximo/possível (aqui só
// garante que a janela fique visível dentro do monitor dado).
inline WindowBounds revalidate(const WindowBounds& saved, const MonitorRect& mon) {
    WindowBounds b = saved;
    b.w = std::min(b.w, mon.w);
    b.h = std::min(b.h, mon.h);
    b.x = std::max(mon.x, std::min(b.x, mon.x + mon.w - b.w));
    b.y = std::max(mon.y, std::min(b.y, mon.y + mon.h - b.h));
    return b;
}

}  // namespace rasgo::panel
