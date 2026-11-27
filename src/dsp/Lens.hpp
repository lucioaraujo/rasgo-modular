#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

// LENS (#95) — foco e desfoque do espectro (ideia óptica, 9/11/2026,
// aprovada pelo autor: "aprovo todos"; dossiê `dossies/95_lens.md`).
//
// Uma lente de câmera para o som. O espectro é dividido em 8 faixas (de
// 70 Hz a 11 kHz). A faixa em FOCUS fica nítida; as outras se BORRAM no
// tempo — passam por difusores (três passa-tudo de Schroeder em série), e o
// ataque delas se espalha como uma imagem fora de foco. DEPTH é a
// PROFUNDIDADE DE CAMPO, a abertura do diafragma: pequena, só a faixa do
// foco fica nítida (fundo desfocado, retrato); grande, quase tudo fica
// nítido (paisagem). BLUR é o quanto o desfocado borra.
//
// Transparente: com BLUR 0 (ou tudo dentro da profundidade de campo), a
// saída é a entrada, amostra a amostra — a lente só tira de foco o que está
// fora dele (saída = entrada + Σ (faixa difundida − faixa)).
//
// Território próprio do Rasgo: o Cardinal não tem módulos ópticos.
//
// Desvio Rasgo: o desfoque abre o estéreo — os difusores de L e R têm
// tempos diferentes, então o que sai de foco também se alarga, como o
// bokeh que se espalha no fundo da foto; o que está em foco fica no centro.
//
// prepare() aloca as linhas; process() não aloca.
namespace rasgo::modular {

class Lens final : public Signal {
public:
    static constexpr int kBands = 8;
    static constexpr int kStages = 3;

    Lens() : Signal(
        {{"in", PortKind::Audio, ""},
         {"focus_cv", PortKind::Control, "v/oct"},
         {"blur_cv", PortKind::Control, ""}},
        {{"l", PortKind::Audio, ""},
         {"r", PortKind::Audio, ""}},
        {{"focus", 60.0f, 8000.0f, 800.0f, "Hz"},
         {"blur", 0.0f, 1.0f, 0.6f, ""},
         {"depth", 0.25f, 4.0f, 1.0f, "oct"},
         {"mix", 0.0f, 1.0f, 1.0f, ""}}) {}

    std::string type() const override { return "LENS"; }

    Panel panel() const override {
        Panel p;
        // Personalização (autor, 27/11/2026: "avance na personalização dos
        // módulos"; jacks embaixo): o anel de
        // foco da lente, grande no centro; BLUR e DEPTH nos lados.
        p.hp = 10;
        p.add(Widget::Kind::Label, "LENS", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "lens", "", 2.5f, 6.0f, 45.80f);
        p.add(Widget::Kind::Knob, "BLUR", "blur", 5.40f, 31.0f);
        p.add(Widget::Kind::Knob, "FOCUS", "focus", 18.40f, 27.0f, 14.0f);   // o herói
        p.add(Widget::Kind::Knob, "DEPTH", "depth", 37.40f, 31.0f);
        p.add(Widget::Kind::Knob, "MIX", "mix", 21.40f, 52.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 12.70f, 92.0f);
        p.add(Widget::Kind::Jack, "FOC", "in:focus_cv", 25.40f, 92.0f);
        p.add(Widget::Kind::Jack, "BLUR", "in:blur_cv", 38.10f, 92.0f);
        p.add(Widget::Kind::Jack, "L", "out:l", 19.05f, 110.0f);
        p.add(Widget::Kind::Jack, "R", "out:r", 31.75f, 110.0f);
        return p;
    }

    static float bandHz(int b) noexcept {   // centros em ~0,92 oitava: 70 Hz … 11 kHz
        return 70.0f * std::pow(2.0f, 0.92f * static_cast<float>(b));
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        for (int b = 0; b < kBands; ++b) {
            const float fc = std::min(bandHz(b), 0.42f * sr_);
            svfG_[static_cast<std::size_t>(b)] = std::tan(3.14159265f * fc / sr_);
            ic1_[static_cast<std::size_t>(b)] = ic2_[static_cast<std::size_t>(b)] = 0.0f;
            for (int side = 0; side < 2; ++side)
                for (int s = 0; s < kStages; ++s) {
                    // tempos de difusão: mais longos no grave, desencontrados em L e R
                    const float ms = (4.3f + 6.1f * static_cast<float>(s)) * (1.0f + 0.22f * static_cast<float>(kBands - 1 - b) / 7.0f)
                                   * (side == 0 ? 1.0f : 1.17f);
                    auto& line = ap_[idx(b, side, s)];
                    line.buf.assign(static_cast<std::size_t>(std::max(2.0f, ms * 0.001f * sr_)) + 1, 0.0f);
                    line.pos = 0;
                }
            g_[static_cast<std::size_t>(b)] = 0.0f;
        }
    }

    // O quanto a faixa b fica fora de foco (0 = nítida … 1 = toda borrada).
    static float defocus(int b, float focusHz, float depthOct) noexcept {
        const float d = std::fabs(std::log2(bandHz(b) / std::max(20.0f, focusHz)));
        const float half = 0.5f * depthOct;   // metade da profundidade de campo de cada lado
        return std::clamp((d - half) / std::max(0.25f, half + 0.5f), 0.0f, 1.0f);
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 2) return;
        const AudioBlock* in = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* fcv = inputs.size() > 1 ? inputs[1] : nullptr;
        const AudioBlock* bcv = inputs.size() > 2 ? inputs[2] : nullptr;

        const float focusK = std::clamp(safe(parameterValue("focus"), 800.0f), 60.0f, 8000.0f);
        const float blurK = std::clamp(safe(parameterValue("blur"), 0.6f), 0.0f, 1.0f);
        const float depth = std::clamp(safe(parameterValue("depth"), 1.0f), 0.25f, 4.0f);
        const float mix = std::clamp(safe(parameterValue("mix"), 1.0f), 0.0f, 1.0f);
        // os coeficientes andam devagar (sem clique quando o foco se move)
        const float glide = 1.0f - std::exp(-1.0f / (0.03f * sr_));
        constexpr float kQ = 1.3f;
        const float kR = 1.0f / kQ;

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            const float x = in ? safe(in->at(0, f), 0.0f) : 0.0f;
            float focus = focusK;
            if (fcv) focus *= std::exp2(std::clamp(safe(fcv->at(0, f), 0.0f), -5.0f, 5.0f));
            float blur = blurK;
            if (bcv) blur = std::clamp(blur + safe(bcv->at(0, f), 0.0f), 0.0f, 1.0f);
            float addL = 0.0f, addR = 0.0f;
            for (int b = 0; b < kBands; ++b) {
                const std::size_t bi = static_cast<std::size_t>(b);
                // SVF (Zavalishin, TPT): a faixa b
                const float gg = svfG_[bi];
                const float hp = (x - (kR + gg) * ic1_[bi] - ic2_[bi]) / (1.0f + kR * gg + gg * gg);
                const float bp = gg * hp + ic1_[bi];
                ic1_[bi] = gg * hp + bp;
                const float lp = gg * bp + ic2_[bi];
                ic2_[bi] = gg * bp + lp;
                const float band = bp * kR;   // ganho 1 no centro
                const float want = 0.72f * blur * defocus(b, focus, depth);
                g_[bi] += (want - g_[bi]) * glide;
                const float g = g_[bi];
                if (g < 1.0e-4f) continue;   // nítida: passa como está
                for (int side = 0; side < 2; ++side) {
                    float v = band;
                    for (int s = 0; s < kStages; ++s) v = allpass(ap_[idx(b, side, s)], v, g);
                    (side == 0 ? addL : addR) += v - band;
                }
            }
            if (!std::isfinite(addL) || !std::isfinite(addR)) { clearLines(); addL = addR = 0.0f; }
            const float l = x + mix * addL, r = x + mix * addR;
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = l;
                outputs[1].at(c, f) = r;
            }
        }
    }

private:
    struct Line { std::vector<float> buf; std::size_t pos = 0; };
    static std::size_t idx(int b, int side, int s) noexcept {
        return static_cast<std::size_t>((b * 2 + side) * kStages + s);
    }
    // passa-tudo de Schroeder: v = x + g·v[n−D]; y = v[n−D] − g·v
    static float allpass(Line& line, float x, float g) noexcept {
        const float delayed = line.buf[line.pos];
        const float v = x + g * delayed;
        line.buf[line.pos] = v;
        if (++line.pos >= line.buf.size()) line.pos = 0;
        return delayed - g * v;
    }
    void clearLines() noexcept {
        for (auto& l : ap_) std::fill(l.buf.begin(), l.buf.end(), 0.0f);
        ic1_.fill(0.0f); ic2_.fill(0.0f);
    }
    static float safe(float v, float fallback) noexcept { return std::isfinite(v) ? v : fallback; }

    float sr_ = 48000.0f;
    std::array<float, kBands> svfG_{}, ic1_{}, ic2_{}, g_{};
    std::array<Line, kBands * 2 * kStages> ap_{};
};

} // namespace rasgo::modular
