#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

// BLEFE (#76) — o jogo do sinal (2026-11-11; dossiê `dossies/76_blefe.md`;
// estudo da família §4.8.3, aprovado: "gosto de todos"; "pode avançar nos
// módulos restantes").
//
// Conceito do `BluffSignal` do Rasgo Synth Studio (mesmo autor; conceito,
// não código): "blefe exige três coisas: informação incompleta +
// expectativa + risco"; "um número aleatório não possui intenção. o blefe
// possui intenção percebida."
//
// Cada gatilho em IN vira um SINAL — sempre o mesmo pulso, seja verdade ou
// blefe: quem escuta não distingue pelo som. A decisão (verdade ou blefe)
// sai por VERDADE com a probabilidade do knob VERD. Uma JANELA depois, a
// verdade se CONFIRMA (pulso em CONF); o blefe não confirma nada. Na
// resolução, a CONFIANÇA anda (média móvel exponencial, MEM) para 1 ou 0.
// A TRAIÇÃO só existe depois de a confiança ter sido construída (passou de
// 0,65): quando ela cai abaixo do LIM, um pulso em TRAI e a RESPOSTA se
// recusa — RESP fica muda até um RST (o perdão, a reorganização).
//
// Saídas: SIG (o sinal, sempre igual), VERD (a verdade, escondida — para
// quem quiser trapacear), CONF (a confirmação), CNF (a confiança, CV 0–1),
// RESP (um pulso na resolução, com altura = confiança; 0 depois da traição)
// e TRAI (o instante da traição).
//
// Desvio: aqui a traição se desfaz por RST (no Studio é para sempre — lá a
// peça acaba; aqui o patch continua). Determinístico (xorshift semeado).
namespace rasgo::modular {

class Blefe final : public Signal {
public:
    Blefe() : Signal(
        {{"in", PortKind::Control, "trig"},
         {"reset", PortKind::Control, "trig"}},
        {{"signal", PortKind::Control, "trig"},
         {"truth", PortKind::Control, "trig"},
         {"confirm", PortKind::Control, "trig"},
         {"trust", PortKind::Control, ""},
         {"response", PortKind::Control, ""},
         {"betrayal", PortKind::Control, "trig"}},
        {{"truthfulness", 0.0f, 1.0f, 0.7f, ""},
         {"window", 0.05f, 4.0f, 0.5f, "s"},
         {"memory", 0.02f, 0.6f, 0.2f, ""},
         {"threshold", 0.1f, 0.6f, 0.35f, ""},
         {"seed", 0.0f, 999.0f, 0.0f, ""}}) {}

    std::string type() const override { return "BLEFE"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "BLEFE", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "blefe", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "VERD", "truthfulness", 8.0f, 30.0f);
        p.add(Widget::Kind::Knob, "JAN", "window", 23.0f, 30.0f);
        p.add(Widget::Kind::Knob, "MEM", "memory", 38.0f, 30.0f);
        p.add(Widget::Kind::Knob, "LIM", "threshold", 8.0f, 54.0f);
        p.add(Widget::Kind::Knob, "SEED", "seed", 23.0f, 54.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 7.0f, 92.0f);
        p.add(Widget::Kind::Jack, "RST", "in:reset", 19.0f, 92.0f);
        p.add(Widget::Kind::Jack, "SIG", "out:signal", 7.0f, 106.0f);
        p.add(Widget::Kind::Jack, "VERD", "out:truth", 19.0f, 106.0f);
        p.add(Widget::Kind::Jack, "CONF", "out:confirm", 31.0f, 106.0f);
        p.add(Widget::Kind::Jack, "CNF", "out:trust", 7.0f, 120.0f);
        p.add(Widget::Kind::Jack, "RESP", "out:response", 19.0f, 120.0f);
        p.add(Widget::Kind::Jack, "TRAI", "out:betrayal", 31.0f, 120.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        seedUsed_ = -1.0f;
        clearAll();
    }

    float trust() const noexcept { return trust_; }
    bool betrayed() const noexcept { return betrayed_; }
    long signals() const noexcept { return signals_; }
    long truths() const noexcept { return truths_; }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 6) return;
        const AudioBlock* in = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* reset = inputs.size() > 1 ? inputs[1] : nullptr;
        const float truthP = std::clamp(safe(parameterValue("truthfulness"), 0.7f), 0.0f, 1.0f);
        const float window = std::clamp(safe(parameterValue("window"), 0.5f), 0.05f, 4.0f);
        const float alpha = std::clamp(safe(parameterValue("memory"), 0.2f), 0.02f, 0.6f);
        const float lim = std::clamp(safe(parameterValue("threshold"), 0.35f), 0.1f, 0.6f);
        const float seed = std::round(std::clamp(safe(parameterValue("seed"), 0.0f), 0.0f, 999.0f));
        if (seed != seedUsed_) {
            seedUsed_ = seed;
            rng_ = 0x9E3779B97F4A7C15ULL ^ (static_cast<std::uint64_t>(seed) * 0xD1B54A32D192ED03ULL);
            if (rng_ == 0) rng_ = 1;
        }
        const long windowLen = std::max(1L, static_cast<long>(window * sr_));
        const int pulse = std::max(1, static_cast<int>(0.010f * sr_));

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            if (reset) {
                const float r = safe(reset->at(0, f), 0.0f);
                if (prevReset_ < 0.5f && r >= 0.5f) forgive();
                prevReset_ = r;
            }
            if (in) {
                const float x = safe(in->at(0, f), 0.0f);
                if (prevIn_ < 0.5f && x >= 0.5f) {
                    // um sinal novo; se havia um pendente, ele se resolve agora
                    if (pendingLeft_ > 0) resolve(alpha, lim, pulse);
                    pendingTrue_ = uniform() < truthP;
                    pendingLeft_ = windowLen;
                    ++signals_;
                    if (pendingTrue_) { ++truths_; truthLeft_ = pulse; }
                    sigLeft_ = pulse;
                }
                prevIn_ = x;
            }
            if (pendingLeft_ > 0 && --pendingLeft_ == 0) resolve(alpha, lim, pulse);

            const float sig = sigLeft_ > 0 ? 1.0f : 0.0f;
            const float tru = truthLeft_ > 0 ? 1.0f : 0.0f;
            const float con = confLeft_ > 0 ? 1.0f : 0.0f;
            const float rsp = respLeft_ > 0 ? respLevel_ : 0.0f;
            const float bet = betLeft_ > 0 ? 1.0f : 0.0f;
            if (sigLeft_ > 0) --sigLeft_;
            if (truthLeft_ > 0) --truthLeft_;
            if (confLeft_ > 0) --confLeft_;
            if (respLeft_ > 0) --respLeft_;
            if (betLeft_ > 0) --betLeft_;
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = sig;
                outputs[1].at(c, f) = tru;
                outputs[2].at(c, f) = con;
                outputs[3].at(c, f) = trust_;
                outputs[4].at(c, f) = rsp;
                outputs[5].at(c, f) = bet;
            }
        }
    }

private:
    static float safe(const float v, const float fb) noexcept { return std::isfinite(v) ? v : fb; }

    float uniform() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        return static_cast<float>((rng_ * 0x2545F4914F6CDD1DULL) >> 40) / 16777216.0f;
    }

    void clearAll() noexcept {
        trust_ = 0.5f;
        everTrusted_ = betrayed_ = pendingTrue_ = false;
        pendingLeft_ = 0;
        sigLeft_ = truthLeft_ = confLeft_ = respLeft_ = betLeft_ = 0;
        respLevel_ = 0.0f;
        signals_ = truths_ = 0;
        prevIn_ = prevReset_ = 0.0f;
    }

    // o perdão: a confiança volta ao neutro, a recusa acaba
    void forgive() noexcept {
        trust_ = 0.5f;
        everTrusted_ = false;
        betrayed_ = false;
        pendingLeft_ = 0;
    }

    void resolve(const float alpha, const float lim, const int pulse) noexcept {
        pendingLeft_ = 0;
        if (pendingTrue_) confLeft_ = pulse;
        trust_ += alpha * ((pendingTrue_ ? 1.0f : 0.0f) - trust_);
        trust_ = std::clamp(trust_, 0.0f, 1.0f);
        if (trust_ > 0.65f) everTrusted_ = true;
        if (!betrayed_ && everTrusted_ && trust_ < lim) {
            betrayed_ = true;
            betLeft_ = pulse;
        }
        respLevel_ = betrayed_ ? 0.0f : trust_;
        respLeft_ = pulse;
    }

    float sr_ = 48000.0f;
    std::uint64_t rng_ = 1;
    float seedUsed_ = -1.0f;
    float trust_ = 0.5f;
    bool everTrusted_ = false, betrayed_ = false, pendingTrue_ = false;
    long pendingLeft_ = 0;
    int sigLeft_ = 0, truthLeft_ = 0, confLeft_ = 0, respLeft_ = 0, betLeft_ = 0;
    float respLevel_ = 0.0f;
    long signals_ = 0, truths_ = 0;
    float prevIn_ = 0.0f, prevReset_ = 0.0f;
};

}  // namespace rasgo::modular
