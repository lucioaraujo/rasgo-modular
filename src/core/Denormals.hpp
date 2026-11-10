#pragma once

// ============================================================================
// Denormais — FTZ/DAZ durante o processamento do grafo (10/11/2026)
// ============================================================================
//
// As caudas dos filtros IIR, dos ressoadores e dos atrasos com realimentação
// decaem no silêncio até ~1e-38, onde os números ficam "desnormalizados" e a
// CPU x86 os processa até 100× mais devagar. Medido no Trioio (irmão RASGO,
// `TRIOIO/docs/REFERENCIAS_POR_MODULO.md` §Denormals): a mistura de uma
// resposta caiu de 12,9 s para 1,1 s com FTZ/DAZ; no Aquorbium, o pior bloco
// caiu de 122 % para 52 % do orçamento. O Modular não tinha proteção
// nenhuma — candidata aos estalos (xruns) com o VARIA ligado.
//
// `ScopedNoDenormals` liga flush-to-zero e denormals-are-zero na thread
// atual e RESTAURA o estado anterior ao sair (o registro de controle é por
// thread: quem chama de fora, um teste ou o host, não fica alterado).
// x86 com SSE: MXCSR bits FTZ (15) e DAZ (6). ARM64: FPCR bit FZ (24).
// Outras arquiteturas: não faz nada.
//
// Conceito e números do Trioio; código próprio (duas instruções do
// processador, técnica pública documentada pela Intel e pela ARM).

#include <cstdint>

#if defined(__SSE__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 1)
#include <xmmintrin.h>
#define RASGO_DENORMALS_SSE 1
#endif

namespace rasgo::modular {

class ScopedNoDenormals {
public:
    ScopedNoDenormals() noexcept {
#if defined(RASGO_DENORMALS_SSE)
        saved_ = _mm_getcsr();
        _mm_setcsr(static_cast<unsigned int>(saved_) | 0x8040u);   // FTZ | DAZ
#elif defined(__aarch64__)
        std::uint64_t fpcr;
        __asm__ volatile("mrs %0, fpcr" : "=r"(fpcr));
        saved_ = fpcr;
        fpcr |= (1ull << 24);           // FZ
        __asm__ volatile("msr fpcr, %0" : : "r"(fpcr));
#endif
    }
    ~ScopedNoDenormals() {
#if defined(RASGO_DENORMALS_SSE)
        _mm_setcsr(static_cast<unsigned int>(saved_));
#elif defined(__aarch64__)
        __asm__ volatile("msr fpcr, %0" : : "r"(saved_));
#endif
    }
    ScopedNoDenormals(const ScopedNoDenormals&) = delete;
    ScopedNoDenormals& operator=(const ScopedNoDenormals&) = delete;

private:
    std::uint64_t saved_ = 0;
};

}  // namespace rasgo::modular
