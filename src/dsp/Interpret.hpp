#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

// INTERPRET (#87) — o intérprete melódico, com CHARACTER (2026-11-11; dossiê
// `dossies/87_interpret.md`; estudo da família §4.5b, aprovado; "avance em
// todos eles"). Conceito do `MelodicInterpreter` do Antitotem (AGPL, mesmo
// autor: articulação legato/staccato/marcato/tenuto, glide contínuo, ápice
// pelo registro, fôlego separado do fraseado, o vetor de caráter de três eixos
// que derivam devagar) — conceito, não código.
//
// Desvio obrigatório: lá o intérprete GERA a própria altura a partir da
// atividade; aqui ele INTERPRETA as notas que chegam (PTCH + GATE de qualquer
// sequenciador) e devolve a nota TOCADA:
//   PTCH — a altura com glide (mais longo nos saltos maiores), overshoot nos
//          saltos (passa do alvo e assenta em ~60 ms) e vibrato progressivo
//          nas notas longas (começa depois de 0,25 s e cresce);
//   AMP  — a articulação: o gesto de cada nota é escolhido pelo CONTEXTO, não
//          por sorteio — salto grande → marcato; ápice (a mais aguda das 8
//          recentes) → tenuto; duas notas rápidas seguidas (< 0,18 s) →
//          staccato; o resto →
//          legato; com fôlego: numa frase longa sem pausa o nível cede até 15 %
//          e volta na pausa;
//   BRT  — o brilho: registro em relação ao recente, ápice e o eixo BRILHO;
//   FRASE — o estado da frase (calma 0 · subida ⅓ · clímax ⅔ · repouso 1);
//   GT   — o gate articulado (o staccato encurta).
// CHARACTER: três eixos (ENERGIA, SUAVIDADE, BRILHO) que DERIVAM devagar
// (DERIVA: ±0,25, períodos de dezenas de segundos) — mais lentos que a frase:
// um clima que dura várias frases. ENERGIA → mais marcato, vibrato maior,
// overshoot maior; SUAVIDADE → glides longos, legato; BRILHO → BRT. A entrada
// CAR soma à ENERGIA. EXPR é o quanto tudo isso age (0 = a nota crua).
// Determinístico (deriva por senoides incomensuráveis), sem alocação.
namespace rasgo::modular {

class Interpret final : public Signal {
public:
    enum class Gesture { Legato, Staccato, Marcato, Tenuto };

    Interpret() : Signal(
        {{"pitch", PortKind::Control, "v/oct"},
         {"gate", PortKind::Control, "gate"},
         {"character", PortKind::Control, ""}},
        {{"pitch", PortKind::Control, "v/oct"},
         {"amp", PortKind::Control, ""},
         {"bright", PortKind::Control, ""},
         {"phrase", PortKind::Control, ""},
         {"gate", PortKind::Control, "gate"}},
        {{"expression", 0.0f, 1.0f, 0.7f, ""},
         {"glide", 0.0f, 0.3f, 0.04f, "s"},
         {"vibrato", 0.0f, 1.0f, 0.4f, ""},
         {"energy", 0.0f, 1.0f, 0.5f, ""},
         {"softness", 0.0f, 1.0f, 0.5f, ""},
         {"brightness", 0.0f, 1.0f, 0.5f, ""},
         {"drift", 0.0f, 1.0f, 0.3f, ""}}) {}

    std::string type() const override { return "INTERPRET"; }

    Panel panel() const override {
        Panel p;
        p.hp = 12;
        p.add(Widget::Kind::Label, "INTERPRET", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "interpret", "", 2.5f, 6.0f, 56.0f);
        p.add(Widget::Kind::Knob, "EXPR", "expression", 7.0f, 28.0f);
        p.add(Widget::Kind::Knob, "GLIDE", "glide", 22.0f, 28.0f);
        p.add(Widget::Kind::Knob, "VIB", "vibrato", 37.0f, 28.0f);
        p.add(Widget::Kind::Knob, "ENERG", "energy", 7.0f, 48.0f);
        p.add(Widget::Kind::Knob, "SUAV", "softness", 22.0f, 48.0f);
        p.add(Widget::Kind::Knob, "BRILHO", "brightness", 37.0f, 48.0f);
        p.add(Widget::Kind::Knob, "DERIVA", "drift", 52.0f, 48.0f);
        p.add(Widget::Kind::Jack, "PTCH", "in:pitch", 7.0f, 88.0f);
        p.add(Widget::Kind::Jack, "GATE", "in:gate", 19.0f, 88.0f);
        p.add(Widget::Kind::Jack, "CAR", "in:character", 31.0f, 88.0f);
        p.add(Widget::Kind::Jack, "PTCH", "out:pitch", 7.0f, 108.0f);
        p.add(Widget::Kind::Jack, "AMP", "out:amp", 19.0f, 108.0f);
        p.add(Widget::Kind::Jack, "BRT", "out:bright", 31.0f, 108.0f);
        p.add(Widget::Kind::Jack, "FRASE", "out:phrase", 43.0f, 108.0f);
        p.add(Widget::Kind::Jack, "GT", "out:gate", 55.0f, 108.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        cur_ = target_ = 0.0f;
        over_ = 0.0f;
        amp_ = 0.0f;
        gateHigh_ = false;
        prevGate_ = 0.0f;
        prevIoi_ = 1.0e9;
        noteAge_ = 1.0e9;
        sinceOn_ = 1.0e9;
        silence_ = 1.0e9;
        vibPhase_ = 0.0;
        t_ = 0.0;
        recent_.fill(0.0f);
        nRecent_ = head_ = 0;
        regAvg_ = 0.0f;
        notesInPhrase_ = 0;
        gesture_ = Gesture::Legato;
        apex_ = false;
        phrase_ = 0.0f;
        lastDir_ = 0.0f;
        artGate_ = false;
    }

    Gesture gesture() const noexcept { return gesture_; }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 5) return;
        const AudioBlock* pin = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* gin = inputs.size() > 1 ? inputs[1] : nullptr;
        const AudioBlock* cin = inputs.size() > 2 ? inputs[2] : nullptr;
        const float expr = knob("expression", 0.7f);
        const float glideBase = std::clamp(safe(parameterValue("glide"), 0.04f), 0.0f, 0.3f);
        const float vibK = knob("vibrato", 0.4f);
        const float energyK = knob("energy", 0.5f);
        const float softK = knob("softness", 0.5f);
        const float brightK = knob("brightness", 0.5f);
        const float driftK = knob("drift", 0.3f);
        const double dt = 1.0 / sr_;

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            t_ += dt;
            // CHARACTER: os três eixos derivam devagar (senoides
            // incomensuráveis, períodos de 23 a 71 s)
            const float dE = driftK * 0.25f * static_cast<float>(std::sin(t_ * 0.27) * 0.6 + std::sin(t_ * 0.089) * 0.4);
            const float dS = driftK * 0.25f * static_cast<float>(std::sin(t_ * 0.19 + 1.7) * 0.6 + std::sin(t_ * 0.11 + 0.4) * 0.4);
            const float dB = driftK * 0.25f * static_cast<float>(std::sin(t_ * 0.23 + 3.1) * 0.6 + std::sin(t_ * 0.071 + 2.2) * 0.4);
            const float energy = std::clamp(energyK + dE + (cin ? safe(cin->at(0, f), 0.0f) : 0.0f), 0.0f, 1.0f);
            const float soft = std::clamp(softK + dS, 0.0f, 1.0f);
            const float bright = std::clamp(brightK + dB, 0.0f, 1.0f);
            energy_ = energy; soft_ = soft;

            const float p = pin ? safe(pin->at(0, f), 0.0f) : target_;
            const float g = gin ? safe(gin->at(0, f), 0.0f) : 0.0f;
            const bool rising = prevGate_ < 0.5f && g >= 0.5f;
            const bool falling = prevGate_ >= 0.5f && g < 0.5f;
            prevGate_ = g;
            sinceOn_ += dt;
            if (!gateHigh_) silence_ += dt;
            if (rising) onNote(p, expr);
            if (falling) { gateHigh_ = false; silence_ = 0.0; }
            if (gateHigh_) { noteAge_ += dt; silence_ = 0.0; }
            // fôlego: uma pausa de 0,4 s fecha a frase e devolve o ar
            if (silence_ > 0.4) notesInPhrase_ = 0;

            // altura: glide (mais longo nos saltos e com SUAVIDADE)
            const float interval = std::fabs(target_ - startPitch_);
            const float glideT = expr * glideBase * (1.0f + 3.0f * std::min(1.0f, interval)) * (0.5f + soft);
            const float coef = glideT > 1.0e-4f ? 1.0f - std::exp(-1.0f / (glideT * static_cast<float>(sr_))) : 1.0f;
            // o overshoot entra no ALVO do glide e se desfaz um pouco mais
            // devagar que o próprio glide: a altura passa do alvo e assenta
            cur_ += (target_ + over_ - cur_) * coef;
            over_ *= std::exp(-1.0f / ((std::max(0.06f, 1.5f * glideT)) * static_cast<float>(sr_)));
            // vibrato progressivo: só nas notas longas, depois de 0,25 s
            float vib = 0.0f;
            if (gateHigh_ && gesture_ != Gesture::Staccato && noteAge_ > 0.25) {
                const float grow = std::min(1.0f, static_cast<float>((noteAge_ - 0.25) / 0.5));
                const float depthCents = expr * vibK * 30.0f * (0.3f + 0.7f * energy)
                                       * (gesture_ == Gesture::Tenuto ? 1.3f : 1.0f);
                vibPhase_ += 2.0 * 3.14159265358979 * (5.0 + energy) * dt;
                vib = grow * depthCents / 1200.0f * static_cast<float>(std::sin(vibPhase_));
            }
            const float pitchOut = cur_ + over_ + vib;

            // articulação
            const float sr = static_cast<float>(sr_);
            float targetAmp = 0.0f;
            if (gateHigh_) {
                const float age = static_cast<float>(noteAge_);
                switch (gesture_) {
                case Gesture::Staccato: targetAmp = age < 0.08f ? 1.0f : 0.1f; break;
                case Gesture::Marcato: targetAmp = age < 0.03f ? 1.0f : 0.72f; break;
                case Gesture::Tenuto: targetAmp = 0.95f; break;
                default: targetAmp = 0.85f; break;
                }
                // fôlego: a frase longa cede até 15 %
                targetAmp *= 1.0f - expr * 0.15f * std::min(1.0f, static_cast<float>(notesInPhrase_) / 16.0f);
                targetAmp = (1.0f - expr) * 1.0f + expr * targetAmp;
            }
            const float attack = gesture_ == Gesture::Marcato ? 0.002f : (0.004f + 0.02f * soft * expr);
            const float release = gesture_ == Gesture::Legato ? 0.2f : 0.08f;
            const float tc = targetAmp > amp_ ? attack : (gateHigh_ ? 0.06f : release);
            amp_ += (targetAmp - amp_) * (1.0f - std::exp(-1.0f / (tc * sr)));
            artGate_ = gateHigh_ && !(gesture_ == Gesture::Staccato && noteAge_ > 0.08);

            const float brt = std::clamp(0.3f + 0.6f * (target_ - regAvg_) + 0.4f * (bright - 0.5f)
                                         + (apex_ ? 0.2f : 0.0f), 0.0f, 1.0f);
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = pitchOut;
                outputs[1].at(c, f) = amp_;
                outputs[2].at(c, f) = brt;
                outputs[3].at(c, f) = phrase_;
                outputs[4].at(c, f) = artGate_ ? 1.0f : 0.0f;
            }
        }
    }

private:
    static float safe(const float v, const float fb) noexcept { return std::isfinite(v) ? v : fb; }
    float knob(const char* id, const float fb) const noexcept {
        return std::clamp(safe(parameterValue(id), fb), 0.0f, 1.0f);
    }

    void onNote(const float p, const float expr) noexcept {
        const float d = p - target_;
        const float ad = std::fabs(d) * 12.0f;   // semitons
        // ápice: mais aguda (½ semitom de folga) que as 8 recentes
        float hi = -1.0e9f;
        for (int i = 0; i < nRecent_; ++i) hi = std::max(hi, recent_[static_cast<std::size_t>(i)]);
        apex_ = nRecent_ > 0 && p > hi + 0.5f / 12.0f;
        const double ioi = sinceOn_;
        // o gesto pelo contexto (ENERGIA puxa ao marcato, SUAVIDADE ao legato)
        if (ad >= 5.0f && energy_ > 0.3f * soft_ + 0.15f) gesture_ = Gesture::Marcato;
        else if (apex_) gesture_ = Gesture::Tenuto;
        // staccato: DUAS notas rápidas seguidas (uma só não prevê a próxima)
        else if (ioi < 0.18 && prevIoi_ < 0.18 && soft_ < 0.75f) gesture_ = Gesture::Staccato;
        else gesture_ = Gesture::Legato;
        // overshoot nos saltos (≥ 3 st): passa do alvo e assenta
        over_ = ad >= 3.0f ? expr * 0.15f * d * (0.4f + energy_) : 0.0f;
        // a frase: subida, clímax (ápice), repouso (desce depois do ápice)
        if (apex_) phrase_ = 2.0f / 3.0f;
        else if (d > 0.0f && lastDir_ >= 0.0f) phrase_ = 1.0f / 3.0f;
        else if (d < 0.0f && phrase_ >= 1.0f / 3.0f) phrase_ = 1.0f;
        else if (std::fabs(d) < 1.0e-4f) phrase_ = 0.0f;
        lastDir_ = d;
        prevIoi_ = ioi;
        startPitch_ = target_;
        target_ = p;
        if (nRecent_ == 0 && notesTotal_ == 0) { cur_ = p; startPitch_ = p; over_ = 0.0f; }
        recent_[static_cast<std::size_t>(head_)] = p;
        head_ = (head_ + 1) % 8;
        nRecent_ = std::min(nRecent_ + 1, 8);
        regAvg_ = notesTotal_ == 0 ? p : 0.85f * regAvg_ + 0.15f * p;
        ++notesTotal_;
        ++notesInPhrase_;
        gateHigh_ = true;
        noteAge_ = 0.0;
        sinceOn_ = 0.0;
        vibPhase_ = 0.0;
    }

    double sr_ = 48000.0;
    float cur_ = 0.0f, target_ = 0.0f, startPitch_ = 0.0f, over_ = 0.0f, amp_ = 0.0f;
    bool gateHigh_ = false, artGate_ = false, apex_ = false;
    float prevGate_ = 0.0f;
    double noteAge_ = 1.0e9, sinceOn_ = 1.0e9, silence_ = 1.0e9, vibPhase_ = 0.0, t_ = 0.0;
    std::array<float, 8> recent_{};
    int nRecent_ = 0, head_ = 0;
    float regAvg_ = 0.0f;
    int notesInPhrase_ = 0;
    long notesTotal_ = 0;
    Gesture gesture_ = Gesture::Legato;
    float phrase_ = 0.0f, lastDir_ = 0.0f;
    float energy_ = 0.5f, soft_ = 0.5f;
    double prevIoi_ = 1.0e9;
};

}  // namespace rasgo::modular
