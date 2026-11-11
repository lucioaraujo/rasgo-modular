#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>

// ACCENT (#75) — o campo de acento (2026-11-11; dossiê `dossies/75_accent.md`,
// aprovado pelo autor com as quatro decisões: agógico e síncope agora,
// saída em níveis, knob ROT, nome ACCENT/ACENTO).
//
// Várias CAUSAS somadas numa ÊNFASE contínua (0–1), entregue em várias
// saídas ao mesmo tempo — o acento como gesto (mais forte, mais brilhante,
// mais curto), não como volume. Taxonomia: acento fenomenal / métrico /
// estrutural (Lerdahl & Jackendoff, 1983); estruturas combinadas (Drake &
// Palmer, 1993); agógico (tradição de H. Riemann).
//
// Causas, cada uma com peso: MÉTR (posição num ciclo de CICLO notas, peso
// mdc(pos, ciclo)/ciclo — 2-ádico num ciclo de 8, 3+3 num de 6), SALTO
// (intervalo desde a nota anterior, 1 oitava = inteiro), ÁPICE (a nota mais
// aguda das últimas 8), EXT (a entrada ACC), AGÓG (a nota anterior foi longa
// em relação à média — só dá para acentuar a SEGUINTE), SÍNC (acento de fora
// num tempo fraco). Combinação por "ou" probabilístico: 1 − Π(1 − wᵢ·cᵢ) —
// causas coincidentes se reforçam e o resultado nunca passa de 1.
//
// HERANÇA (Antitotem, `accentTail`): a cauda de um acento passa à nota
// seguinte e cai pela metade a cada nota. SUPR: depois de uma ênfase alta,
// a seguinte perde essa proporção (refratário). ROT (Antitotem,
// `accentRotation`): o tempo forte migra uma posição a cada tantos ciclos.
//
// Saídas (seguram até a próxima nota, exceto SWP e GT): ÊNF; VCA (ganho
// 1 − 0,4·PROF·(1 − ênf), o 0,6 + 0,4·acc do `AcidBasslineVoice` do Studio);
// FC (+2·PROF·ênf oitavas); DEC (−2·PROF·ênf, V/oct para o TIME de um
// ENVELOPE: o acento encurta, como na 303); SWP (o empilhamento da 303:
// cada nota sobe 0,6·ênf do que falta até 1 numa carga que vaza com
// τ 0,25 s — acentos seguidos sobem aos poucos, um sozinho fica em 0,6); GT (o gate passa só com ênf ≥ 0,5); NÍV
// (a ênfase em degraus: 0, ⅓, ⅔, 1).
//
// Conceitos do Antitotem, do Aquorbium (`geometricAccent`, um campo para
// vários destinos) e do Studio, todos do mesmo autor — conceito, não código.
// Determinístico, sem alocação.
namespace rasgo::modular {

class Accent final : public Signal {
public:
    static constexpr int kApexWindow = 8;

    Accent() : Signal(
        {{"gate", PortKind::Control, "gate"},
         {"pitch", PortKind::Control, "v/oct"},
         {"accent", PortKind::Control, ""},
         {"reset", PortKind::Control, "trig"}},
        {{"emphasis", PortKind::Control, ""},
         {"vca", PortKind::Control, ""},
         {"cutoff", PortKind::Control, "v/oct"},
         {"decay", PortKind::Control, "v/oct"},
         {"sweep", PortKind::Control, ""},
         {"gate", PortKind::Control, "gate"},
         {"level", PortKind::Control, ""}},
        {{"metric", 0.0f, 1.0f, 0.6f, ""},
         {"leap", 0.0f, 1.0f, 0.4f, ""},
         {"apex", 0.0f, 1.0f, 0.4f, ""},
         {"external", 0.0f, 1.0f, 1.0f, ""},
         {"agogic", 0.0f, 1.0f, 0.3f, ""},
         {"synco", 0.0f, 1.0f, 0.3f, ""},
         {"cycle", 2.0f, 16.0f, 4.0f, ""},
         {"rotate", 0.0f, 1.0f, 0.0f, ""},
         {"inherit", 0.0f, 1.0f, 0.3f, ""},
         {"suppress", 0.0f, 1.0f, 0.3f, ""},
         {"depth", 0.0f, 1.0f, 0.6f, ""}}) {}

    std::string type() const override { return "ACCENT"; }

    Panel panel() const override {
        Panel p;
        p.hp = 14;
        p.add(Widget::Kind::Label, "ACCENT", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "accent", "", 2.5f, 6.0f, 66.1f);
        p.add(Widget::Kind::Knob, "METR", "metric", 9.0f, 28.0f);
        p.add(Widget::Kind::Knob, "SALTO", "leap", 25.0f, 28.0f);
        p.add(Widget::Kind::Knob, "APICE", "apex", 41.0f, 28.0f);
        p.add(Widget::Kind::Knob, "EXT", "external", 57.0f, 28.0f);
        p.add(Widget::Kind::Knob, "AGOG", "agogic", 9.0f, 46.0f);
        p.add(Widget::Kind::Knob, "SINC", "synco", 25.0f, 46.0f);
        p.add(Widget::Kind::Knob, "CICLO", "cycle", 41.0f, 46.0f);
        p.add(Widget::Kind::Knob, "ROT", "rotate", 57.0f, 46.0f);
        p.add(Widget::Kind::Knob, "HER", "inherit", 9.0f, 64.0f);
        p.add(Widget::Kind::Knob, "SUPR", "suppress", 25.0f, 64.0f);
        p.add(Widget::Kind::Knob, "PROF", "depth", 41.0f, 64.0f);
        p.add(Widget::Kind::Jack, "GATE", "in:gate", 7.0f, 88.0f);
        p.add(Widget::Kind::Jack, "PTCH", "in:pitch", 19.0f, 88.0f);
        p.add(Widget::Kind::Jack, "ACC", "in:accent", 31.0f, 88.0f);
        p.add(Widget::Kind::Jack, "RST", "in:reset", 43.0f, 88.0f);
        p.add(Widget::Kind::Jack, "ENF", "out:emphasis", 7.0f, 104.0f);
        p.add(Widget::Kind::Jack, "VCA", "out:vca", 19.0f, 104.0f);
        p.add(Widget::Kind::Jack, "FC", "out:cutoff", 31.0f, 104.0f);
        p.add(Widget::Kind::Jack, "DEC", "out:decay", 43.0f, 104.0f);
        p.add(Widget::Kind::Jack, "SWP", "out:sweep", 7.0f, 118.0f);
        p.add(Widget::Kind::Jack, "GT", "out:gate", 19.0f, 118.0f);
        p.add(Widget::Kind::Jack, "NIV", "out:level", 31.0f, 118.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        clearAll();
    }

    // Para os testes.
    float emphasis() const noexcept { return emph_; }
    long notes() const noexcept { return count_; }
    int rotation() const noexcept { return rot_; }

    // Peso métrico da posição `pos` num ciclo de `len` notas: 1 no tempo
    // forte; senão mdc(pos, len)/len (num ciclo de 8: 5º = ½; 3º e 7º = ¼).
    static float metricWeight(const int pos, const int len) noexcept {
        if (len <= 1 || pos % len == 0) return 1.0f;
        return static_cast<float>(std::gcd(pos % len, len)) / static_cast<float>(len);
    }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 7) return;
        const AudioBlock* gate = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* pitch = inputs.size() > 1 ? inputs[1] : nullptr;
        const AudioBlock* acc = inputs.size() > 2 ? inputs[2] : nullptr;
        const AudioBlock* reset = inputs.size() > 3 ? inputs[3] : nullptr;

        const Weights w{
            knob("metric", 0.6f), knob("leap", 0.4f), knob("apex", 0.4f),
            knob("external", 1.0f), knob("agogic", 0.3f), knob("synco", 0.3f)};
        const int cycle = static_cast<int>(std::lround(
            std::clamp(safe(parameterValue("cycle"), 4.0f), 2.0f, 16.0f)));
        const float rotate = knob("rotate", 0.0f);
        const float inherit = knob("inherit", 0.3f);
        const float suppress = knob("suppress", 0.3f);
        const float depth = knob("depth", 0.6f);
        const float sweepFade = std::exp(-1.0f / (0.25f * sr_));
        const int accHold = std::max(1, static_cast<int>(0.003f * sr_));   // 3 ms

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            if (reset) {
                const float r = safe(reset->at(0, f), 0.0f);
                if (prevReset_ < 0.5f && r >= 0.5f) clearAll();
                prevReset_ = r;
            }
            // o acento de fora pode chegar uma amostra antes/depois do gate
            // (CLOCK, TRIGSEQ): o pico dos últimos 3 ms
            if (acc) {
                const float a = std::clamp(safe(acc->at(0, f), 0.0f), 0.0f, 1.0f);
                if (a >= accPeak_) { accPeak_ = a; accAge_ = 0; }
                else if (++accAge_ > accHold) { accPeak_ = a; accAge_ = 0; }
            }
            const float g = gate ? safe(gate->at(0, f), 0.0f) : 0.0f;
            const bool on = g >= 0.5f;
            ++sinceOnset_;
            if (on && !gateHigh_) {
                const float p = pitch ? safe(pitch->at(0, f), 0.0f) : 0.0f;
                onNote(p, w, cycle, rotate, inherit, suppress);
            }
            gateHigh_ = on;
            sweep_ *= sweepFade;

            const float vca = 1.0f - 0.4f * depth * (1.0f - emph_);
            const float fc = 2.0f * depth * emph_;
            const float dec = -2.0f * depth * emph_;
            const float gt = (on && emph_ >= 0.5f) ? 1.0f : 0.0f;
            const float lvl = emph_ < 0.25f ? 0.0f
                            : emph_ < 0.5f ? 1.0f / 3.0f
                            : emph_ < 0.75f ? 2.0f / 3.0f : 1.0f;
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = emph_;
                outputs[1].at(c, f) = vca;
                outputs[2].at(c, f) = fc;
                outputs[3].at(c, f) = dec;
                outputs[4].at(c, f) = sweep_;
                outputs[5].at(c, f) = gt;
                outputs[6].at(c, f) = lvl;
            }
        }
    }

private:
    struct Weights { float metric, leap, apex, external, agogic, synco; };

    static float safe(const float v, const float fallback) noexcept {
        return std::isfinite(v) ? v : fallback;
    }
    float knob(const char* id, const float fallback) const noexcept {
        return std::clamp(safe(parameterValue(id), fallback), 0.0f, 1.0f);
    }

    void clearAll() noexcept {
        count_ = 0;
        rot_ = 0;
        cyclesSinceRot_ = 0;
        emph_ = 0.0f;
        tail_ = 0.0f;
        sweep_ = 0.0f;
        accPeak_ = 0.0f;
        accAge_ = 0;
        havePrev_ = false;
        prevPitch_ = 0.0f;
        apexRing_.fill(0.0f);
        apexCount_ = 0;
        apexHead_ = 0;
        sinceOnset_ = 0;
        meanDur_ = 0.0f;
        gateHigh_ = false;
        prevReset_ = 0.0f;
    }

    void onNote(const float pitchV, const Weights& w, const int cycle, const float rotate,
                const float inherit, const float suppress) noexcept {
        // MÉTR: posição no ciclo, com a rotação
        const int pos = static_cast<int>((count_ + rot_) % cycle);
        const float metric = metricWeight(pos, cycle);
        // SALTO: 1 oitava = 1
        const float leap = havePrev_ ? std::min(1.0f, std::fabs(pitchV - prevPitch_)) : 0.0f;
        // ÁPICE: mais aguda (½ semitom de folga) que todas as últimas 8
        float apex = 0.0f;
        if (apexCount_ > 0) {
            float hi = apexRing_[0];
            for (int i = 0; i < apexCount_; ++i) hi = std::max(hi, apexRing_[static_cast<std::size_t>(i)]);
            if (pitchV > hi + 0.5f / 12.0f) apex = 1.0f;
        }
        // AGÓG: a nota anterior durou bem mais que a média → esta é acentuada
        float agogic = 0.0f;
        const float dur = static_cast<float>(sinceOnset_) / sr_;
        if (count_ > 0) {
            if (meanDur_ > 0.0f && dur > 1.5f * meanDur_)
                agogic = std::clamp((dur / meanDur_ - 1.5f) / 1.5f, 0.0f, 1.0f);
            meanDur_ = meanDur_ <= 0.0f ? dur : 0.8f * meanDur_ + 0.2f * dur;
        }
        // EXT e SÍNC (o acento de fora num tempo fraco)
        const float ext = accPeak_;
        const float synco = ext * (1.0f - metric);

        // "ou" probabilístico das causas
        float keep = 1.0f;
        keep *= 1.0f - w.metric * metric;
        keep *= 1.0f - w.leap * leap;
        keep *= 1.0f - w.apex * apex;
        keep *= 1.0f - w.external * ext;
        keep *= 1.0f - w.agogic * agogic;
        keep *= 1.0f - w.synco * synco;
        const float raw = 1.0f - keep;
        // HERANÇA (a cauda da nota anterior entra pelo mesmo "ou")
        float e = 1.0f - (1.0f - raw) * (1.0f - inherit * tail_);
        tail_ = std::max(0.5f * tail_, raw);
        // SUPRESSÃO: depois de uma ênfase alta, esta perde essa proporção
        e *= 1.0f - suppress * std::max(0.0f, emph_ - 0.3f) / 0.7f;
        emph_ = std::clamp(e, 0.0f, 1.0f);

        // SWP: o empilhamento — cada nota sobe 0,6·ênf do que falta até 1
        // (sozinho: 0,6; seguidos e próximos: 0,79, 0,85… sobe aos poucos)
        sweep_ = std::min(1.0f, sweep_ + 0.6f * emph_ * (1.0f - sweep_));

        // memória
        prevPitch_ = pitchV;
        havePrev_ = true;
        apexRing_[static_cast<std::size_t>(apexHead_)] = pitchV;
        apexHead_ = (apexHead_ + 1) % kApexWindow;
        apexCount_ = std::min(apexCount_ + 1, kApexWindow);
        sinceOnset_ = 0;
        ++count_;
        // ROT: o tempo forte anda uma posição a cada 1 + 15·(1 − ROT) ciclos
        if (count_ % cycle == 0 && rotate > 0.0f) {
            const int every = 1 + static_cast<int>(std::lround(15.0f * (1.0f - rotate)));
            if (++cyclesSinceRot_ >= every) {
                cyclesSinceRot_ = 0;
                rot_ = (rot_ + 1) % 16;
            }
        }
    }

    float sr_ = 48000.0f;
    long count_ = 0;
    int rot_ = 0, cyclesSinceRot_ = 0;
    float emph_ = 0.0f, tail_ = 0.0f, sweep_ = 0.0f;
    float accPeak_ = 0.0f;
    int accAge_ = 0;
    bool havePrev_ = false;
    float prevPitch_ = 0.0f;
    std::array<float, kApexWindow> apexRing_{};
    int apexCount_ = 0, apexHead_ = 0;
    long sinceOnset_ = 0;
    float meanDur_ = 0.0f;
    bool gateHigh_ = false;
    float prevReset_ = 0.0f;
};

}  // namespace rasgo::modular
