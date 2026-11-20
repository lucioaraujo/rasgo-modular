#pragma once

#include <atomic>

namespace rasgo::modular {

// Um float que duas threads podem ler e escrever sem lock (20/11/2026).
// A interface escreve o valor de intenção (knob, VARIA) e o áudio o lê —
// e o próprio áudio escreve o valor modulado. Até aqui isso exigia o
// `gmx` em volta de cada mexida, e cada mexida podia deixar o áudio sem o
// bloco (a "trava tela↔áudio", origem dos estalos com o VARIA). Atômico
// com ordem relaxada: cada leitura é um valor inteiro que alguém escreveu
// (nunca um valor rasgado), sem custo no x86/ARM além de um load/store
// comum. Copiável (copia o valor), para os vetores e os testes.
class RtFloat {
public:
    RtFloat(const float v = 0.0f) noexcept : v_(v) {}
    RtFloat(const RtFloat& o) noexcept : v_(o.load()) {}
    RtFloat& operator=(const RtFloat& o) noexcept { store(o.load()); return *this; }
    RtFloat& operator=(const float v) noexcept { store(v); return *this; }
    operator float() const noexcept { return load(); }
    float load() const noexcept { return v_.load(std::memory_order_relaxed); }
    void store(const float v) noexcept { v_.store(v, std::memory_order_relaxed); }
private:
    std::atomic<float> v_;
};

}  // namespace rasgo::modular
