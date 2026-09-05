#pragma once

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>

// ============================================================================
// TRIGSEQ — Grade de trigs / sequenciador de percussão (Módulo 30)
// ============================================================================
//
// O `CLOCK` faz euclidiano de UMA linha; o `SEQUENCE` faz altura+gate de UMA
// voz. Este é a GRADE MULTIPISTA — 4 linhas de gate on/off tocando juntas, a
// base rítmica de percussão. Seguindo a identidade RASGO ("soa ao carregar",
// alignment not presets), NÃO é um editor de passos: é um GERADOR — uma matriz
// de padrões arquetípicos que o `map` percorre, `density` por linha esculpe,
// `chaos` humaniza (probabilidade, não flip cru), `drift` faz evoluir.
//
// Ver o dossiê: `RASGO_MODULAR/dossies/30_trigseq.md`.
//
// Fontes ESTUDADAS (conceito, não código):
//   - Mutable Grids (mapa rítmico + limiar de densidade sobre o peso do
//     passo) — conceito público, tabelas reescritas aqui;
//   - Roland TR-808/909 (grade 16 passos × instrumentos, acento derivado);
//   - Pamela's PRO Workout / Vermona randomRHYTHM (prob. por passo, fill);
//   - modo browniano do SEQUENCE / spread do DECISION (acaso estruturado).
//
// Determinístico: xorshift semeado em prepare(). Avança por `clock` externo
// OU relógio interno em `rate`.

namespace rasgo::modular {

class TrigSeq final : public Signal {
public:
    TrigSeq()
        : Signal(
              {{"clock", PortKind::Control, "trig"},
               {"reset", PortKind::Control, "trig"},
               {"fill", PortKind::Control, "gate"},
               {"map_cv", PortKind::Control, ""}},
              {{"t1", PortKind::Control, "gate"},
               {"t2", PortKind::Control, "gate"},
               {"t3", PortKind::Control, "gate"},
               {"t4", PortKind::Control, "gate"},
               {"accent", PortKind::Control, "gate"},
               {"any", PortKind::Control, "gate"}},
              {{"length", 2.0f, 16.0f, 16.0f, ""},
               {"rate", 0.1f, 20.0f, 2.0f, "Hz"},
               {"map", 0.0f, 1.0f, 0.3f, ""},
               {"density1", 0.0f, 1.0f, 0.7f, ""},
               {"density2", 0.0f, 1.0f, 0.5f, ""},
               {"density3", 0.0f, 1.0f, 0.6f, ""},
               {"density4", 0.0f, 1.0f, 0.25f, ""},
               {"swing", 0.0f, 1.0f, 0.0f, ""},
               {"chaos", 0.0f, 1.0f, 0.1f, ""},
               {"ratchet", 0.0f, 1.0f, 0.0f, ""},
               {"fill_amt", 0.0f, 1.0f, 0.5f, ""},
               {"drift", 0.0f, 1.0f, 0.0f, ""}}) {}

    std::string type() const override { return "TRIGSEQ"; }

    Panel panel() const override {
        Panel p;
        p.hp = 16;
        p.add(Widget::Kind::Label, "TRIGSEQ", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "grid", "", 2.5f, 6.0f, 76.3f);
        p.add(Widget::Kind::Knob, "LEN", "length", 10.0f, 28.0f);
        p.add(Widget::Kind::Knob, "RATE", "rate", 28.0f, 28.0f);
        p.add(Widget::Kind::Knob, "MAP", "map", 46.0f, 28.0f);
        p.add(Widget::Kind::Knob, "SWING", "swing", 64.0f, 28.0f);
        p.add(Widget::Kind::Knob, "DNS1", "density1", 10.0f, 46.0f);
        p.add(Widget::Kind::Knob, "DNS2", "density2", 28.0f, 46.0f);
        p.add(Widget::Kind::Knob, "DNS3", "density3", 46.0f, 46.0f);
        p.add(Widget::Kind::Knob, "DNS4", "density4", 64.0f, 46.0f);
        p.add(Widget::Kind::Knob, "CHAOS", "chaos", 10.0f, 64.0f);
        p.add(Widget::Kind::Knob, "RATCH", "ratchet", 28.0f, 64.0f);
        p.add(Widget::Kind::Knob, "FILL", "fill_amt", 46.0f, 64.0f);
        p.add(Widget::Kind::Knob, "DRIFT", "drift", 64.0f, 64.0f);
        p.add(Widget::Kind::Jack, "CLK", "in:clock", 7.0f, 86.0f);
        p.add(Widget::Kind::Jack, "RST", "in:reset", 19.0f, 86.0f);
        p.add(Widget::Kind::Jack, "FILL", "in:fill", 31.0f, 86.0f);
        p.add(Widget::Kind::Jack, "MAP", "in:map_cv", 43.0f, 86.0f);
        p.add(Widget::Kind::Jack, "T1", "out:t1", 7.0f, 110.0f);
        p.add(Widget::Kind::Jack, "T2", "out:t2", 19.0f, 110.0f);
        p.add(Widget::Kind::Jack, "T3", "out:t3", 31.0f, 110.0f);
        p.add(Widget::Kind::Jack, "T4", "out:t4", 43.0f, 110.0f);
        p.add(Widget::Kind::Jack, "ACC", "out:accent", 55.0f, 110.0f);
        p.add(Widget::Kind::Jack, "ANY", "out:any", 67.0f, 110.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::max(1.0f, sampleRate);
        step_ = 0;
        phase_ = 0.0f;
        period_ = sr_ / 2.0f;
        periodCount_ = 0.0f;
        prevClock_ = false;
        prevReset_ = false;
        started_ = false;
        pendingCd_ = 0;
        pendingMask_ = 0;
        pendingAccent_ = false;
        pendingAny_ = false;
        accentCd_ = 0;
        anyCd_ = 0;
        mapDrift_ = 0.0f;
        for (int l = 0; l < 4; ++l) {
            gateCd_[l] = 0;
            ratchetLeft_[l] = 0;
            ratchetCd_[l] = 0;
            ratchetIvl_[l] = 0;
            densDrift_[l] = 0.0f;
        }
        rng_ = 0x9E3779B97F4A7C15ULL;
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();

        const int length = clampi(
            static_cast<int>(std::lround(parameterValue("length"))), 2, 16);
        const float rate = std::max(0.01f, parameterValue("rate"));
        const float mapParam = clamp01(parameterValue("map"));
        const float dens[4] = {clamp01(parameterValue("density1")),
                               clamp01(parameterValue("density2")),
                               clamp01(parameterValue("density3")),
                               clamp01(parameterValue("density4"))};
        const float swing = clamp01(parameterValue("swing"));
        const float chaos = clamp01(parameterValue("chaos"));
        const float ratchet = clamp01(parameterValue("ratchet"));
        const float fillAmt = clamp01(parameterValue("fill_amt"));
        const float drift = clamp01(parameterValue("drift"));

        const AudioBlock* clock = inputs[0];
        const AudioBlock* reset = inputs[1];
        const AudioBlock* fillIn = inputs[2];
        const AudioBlock* mapCv = inputs[3];
        const bool externalClock = (clock != nullptr);
        const float intInc = rate / sr_;

        for (std::size_t f = 0; f < frames; ++f) {
            // ---- reset ----
            if (reset != nullptr) {
                const bool hi = reset->at(0, f) >= 0.5f;
                if (hi && !prevReset_) {
                    step_ = 0;
                    phase_ = 0.0f;
                    pendingCd_ = 0;
                    pendingMask_ = 0;
                    for (int l = 0; l < 4; ++l) {
                        gateCd_[l] = 0;
                        ratchetLeft_[l] = 0;
                    }
                    accentCd_ = 0;
                    anyCd_ = 0;
                    started_ = false;
                }
                prevReset_ = hi;
            }

            const bool fillHi =
                fillIn != nullptr && fillIn->at(0, f) >= 0.5f;
            const float mapMod = mapCv != nullptr ? mapCv->at(0, f) : 0.0f;

            // ---- detecta o tique ----
            bool tick = false;
            periodCount_ += 1.0f;
            if (externalClock) {
                const bool hi = clock->at(0, f) >= 0.5f;
                if (hi && !prevClock_) {
                    tick = true;
                    if (started_ && periodCount_ > 1.0f) period_ = periodCount_;
                    periodCount_ = 0.0f;
                }
                prevClock_ = hi;
            } else {
                phase_ += intInc;
                if (phase_ >= 1.0f) {
                    phase_ -= 1.0f;
                    tick = true;
                    period_ = sr_ / rate;
                }
            }

            if (tick) {
                if (started_) step_ = (step_ + 1) % length;
                started_ = true;
                const int s = step_ % length;

                // deriva lenta e limitada
                if (drift > 0.0f) {
                    mapDrift_ = clampf(
                        mapDrift_ + (rng01() * 2.0f - 1.0f) * drift * 0.03f,
                        -0.3f, 0.3f);
                    for (int l = 0; l < 4; ++l)
                        densDrift_[l] = clampf(
                            densDrift_[l]
                                + (rng01() * 2.0f - 1.0f) * drift * 0.02f,
                            -0.2f, 0.2f);
                } else {
                    mapDrift_ *= 0.9f;
                    for (int l = 0; l < 4; ++l) densDrift_[l] *= 0.9f;
                }

                const float pos =
                    clamp01(mapParam + mapMod + mapDrift_) * 3.0f;
                const int c0 = static_cast<int>(pos);
                const int c1 = c0 >= 3 ? 3 : c0 + 1;
                const float g = pos - static_cast<float>(c0);

                std::uint8_t mask = 0;
                int nHits = 0;
                for (int l = 0; l < 4; ++l) {
                    const float w =
                        static_cast<float>(kNode[c0][l][s])
                        + (static_cast<float>(kNode[c1][l][s])
                           - static_cast<float>(kNode[c0][l][s])) * g;
                    float d = dens[l] + densDrift_[l];
                    if (fillHi) d += fillAmt * (1.0f - dens[l]);
                    d = clamp01(d);
                    const float thr = (1.0f - d) * 255.0f;
                    bool fire = w > 0.5f && w >= thr;
                    if (chaos > 0.0f) {
                        const float u = rng01();
                        if (!fire && w > 0.5f
                            && u < chaos * 0.35f * (w / 255.0f)) {
                            fire = true;
                        } else if (fire && w < 250.0f && u < chaos * 0.12f) {
                            fire = false;
                        }
                    }
                    if (fire) {
                        mask |= static_cast<std::uint8_t>(1u << l);
                        ++nHits;
                    }
                }
                const bool accent = nHits >= 2;
                const bool any = nHits >= 1;

                const float delayF =
                    (s & 1) ? swing * 0.66f * period_ : 0.0f;
                const int delay = static_cast<int>(delayF);
                if (delay < 1) {
                    commit(mask, accent, any, ratchet);
                } else {
                    pendingMask_ = mask;
                    pendingAccent_ = accent;
                    pendingAny_ = any;
                    pendingCd_ = delay;
                }
            }

            // ---- swing pendente ----
            if (pendingCd_ > 0) {
                if (--pendingCd_ == 0) {
                    commit(pendingMask_, pendingAccent_, pendingAny_, ratchet);
                    pendingMask_ = 0;
                }
            }

            // ---- ratchet ----
            for (int l = 0; l < 4; ++l) {
                if (ratchetLeft_[l] > 0) {
                    if (--ratchetCd_[l] <= 0) {
                        gateCd_[l] = gateLen();
                        ratchetCd_[l] = ratchetIvl_[l];
                        --ratchetLeft_[l];
                    }
                }
            }

            // ---- saídas ----
            for (int l = 0; l < 4; ++l) {
                const float v = gateCd_[l] > 0 ? 1.0f : 0.0f;
                if (gateCd_[l] > 0) --gateCd_[l];
                for (std::size_t c = 0; c < channels; ++c)
                    outputs[static_cast<std::size_t>(l)].at(c, f) = v;
            }
            const float accV = accentCd_ > 0 ? 1.0f : 0.0f;
            if (accentCd_ > 0) --accentCd_;
            const float anyV = anyCd_ > 0 ? 1.0f : 0.0f;
            if (anyCd_ > 0) --anyCd_;
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[4].at(c, f) = accV;
                outputs[5].at(c, f) = anyV;
            }
        }
    }

private:
    static int clampi(const int v, const int lo, const int hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float clamp01(const float v) noexcept { return clampf(v, 0.0f, 1.0f); }

    // gate curto (trigger) — sempre < período/3 pra as sub-hits do ratchet
    // aparecerem como bordas separadas
    int gateLen() const noexcept {
        const float g = period_ * 0.25f;
        const float lo = 0.003f * sr_;
        const float hi = 0.025f * sr_;
        return static_cast<int>(g < lo ? lo : (g > hi ? hi : g)) + 1;
    }

    float rng01() noexcept {
        rng_ ^= rng_ >> 12;
        rng_ ^= rng_ << 25;
        rng_ ^= rng_ >> 27;
        const std::uint64_t x = rng_ * 0x2545F4914F6CDD1DULL;
        return static_cast<float>((x >> 40) & 0xFFFFFF)
            / static_cast<float>(0x1000000);
    }

    void commit(const std::uint8_t mask, const bool accent, const bool any,
                const float ratchet) noexcept {
        const int gl = gateLen();
        for (int l = 0; l < 4; ++l) {
            if (mask & (1u << l)) {
                gateCd_[l] = gl;
                if (ratchet > 0.0f && rng01() < ratchet * 0.5f) {
                    ratchetLeft_[l] = 2;
                    ratchetIvl_[l] =
                        static_cast<int>(period_ / 3.0f) + 1;
                    ratchetCd_[l] = ratchetIvl_[l];
                }
            }
        }
        if (accent) accentCd_ = gl;
        if (any) anyCd_ = gl;
    }

    // ---- tabelas de caractere: 4 caracteres × 4 linhas × 16 passos ----
    // pesos: 0 mudo · 64 fantasma · 160 normal · 255 downbeat
    // linhas: 0 bumbo · 1 caixa · 2 chimbal · 3 perc
    static constexpr std::uint8_t kNode[4][4][16] = {
        // 0 — straight (rock / house)
        {{255, 0, 0, 0, 0, 0, 160, 0, 255, 0, 0, 0, 0, 0, 160, 0},
         {0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 64, 0, 255, 0, 0, 0},
         {160, 0, 160, 0, 160, 0, 160, 0, 160, 0, 160, 0, 160, 0, 160, 64},
         {0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0, 0, 0, 0, 0, 64}},
        // 1 — broken (breakbeat)
        {{255, 0, 0, 0, 0, 0, 0, 160, 0, 0, 255, 0, 0, 0, 64, 0},
         {0, 0, 0, 0, 255, 0, 0, 0, 0, 64, 0, 0, 255, 0, 64, 0},
         {160, 0, 160, 64, 160, 0, 160, 0, 160, 64, 160, 0, 160, 0, 160, 0},
         {0, 0, 64, 0, 0, 160, 0, 0, 0, 0, 0, 64, 0, 0, 160, 0}},
        // 2 — shuffle (hip-hop / suingado)
        {{255, 0, 0, 64, 0, 0, 160, 0, 0, 0, 255, 0, 0, 64, 0, 0},
         {0, 0, 0, 0, 255, 0, 0, 64, 0, 0, 0, 0, 255, 0, 0, 64},
         {160, 0, 64, 160, 0, 64, 160, 0, 64, 160, 0, 64, 160, 0, 64, 160},
         {0, 64, 0, 0, 0, 0, 0, 160, 0, 64, 0, 0, 0, 0, 160, 0}},
        // 3 — sparse (minimal / dub)
        {{255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 160, 0, 0, 0},
         {0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0},
         {0, 0, 0, 0, 160, 0, 0, 0, 0, 0, 0, 0, 160, 0, 0, 0},
         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 160}}};

    float sr_ = 48000.0f;
    int step_ = 0;
    float phase_ = 0.0f;
    float period_ = 24000.0f;
    float periodCount_ = 0.0f;
    bool prevClock_ = false;
    bool prevReset_ = false;
    bool started_ = false;
    int pendingCd_ = 0;
    std::uint8_t pendingMask_ = 0;
    bool pendingAccent_ = false;
    bool pendingAny_ = false;
    int accentCd_ = 0;
    int anyCd_ = 0;
    int gateCd_[4] = {0, 0, 0, 0};
    int ratchetLeft_[4] = {0, 0, 0, 0};
    int ratchetCd_[4] = {0, 0, 0, 0};
    int ratchetIvl_[4] = {0, 0, 0, 0};
    float mapDrift_ = 0.0f;
    float densDrift_[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    std::uint64_t rng_ = 0x9E3779B97F4A7C15ULL;
};

}  // namespace rasgo::modular
