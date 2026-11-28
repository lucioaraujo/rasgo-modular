#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

// SEASON (#100) — dia, noite e estação (leitura da família, 9/11/2026,
// aprovada pelo autor: "aprovo todos, continue"; dossiê
// `dossies/100_season.md`).
//
// Um relógio lentíssimo, o do Aquorbium (dia, noite e estação escolhem o
// modo e o andamento), feito com a astronomia de verdade: a altura do Sol
// sai de sen h = sen φ·sen δ + cos φ·cos δ·cos H, com a declinação δ do Sol
// variando ±23,44° ao longo do ano e a LATITUDE φ do lugar. Assim, a duração
// do dia muda com a estação, mais quanto mais longe do equador: no equador,
// 12 h o ano inteiro; a 60°, dias longos de verão e curtos de inverno; acima
// do círculo polar (66,6°), o sol da meia-noite e a noite polar.
//
// DAY é a duração de um dia, em segundos; YEAR, quantos dias tem o ano.
// Saídas: SUN (a altura do Sol, 0 na noite, 1 no zênite), LIGHT (gate
// enquanto é dia), DAWN e DUSK (pulsos no nascer e no pôr), YEAR (a fase do
// ano, 0 = equinócio de primavera), SCL (o modo da estação, no formato do
// QUANTIZER: primavera Lídio, verão Maior, outono Dórico, inverno Eólio) e
// TEMPO (0–1: mais rápido ao meio-dia do verão, mais lento na noite do
// inverno). Dá FORMA LONGA ao patch: o andamento e o modo mudam com a luz.
//
// Desvio Rasgo: HEMI — o hemisfério. No sul (Brasil), as estações se
// invertem: o Natal cai no verão.
//
// process() não aloca; determinístico.
namespace rasgo::modular {

class Season final : public Signal {
public:
    Season() : Signal(
        {{"reset", PortKind::Control, "trig"},
         {"speed_cv", PortKind::Control, ""}},
        {{"sun", PortKind::Control, ""},
         {"light", PortKind::Control, "gate"},
         {"dawn", PortKind::Control, "gate"},
         {"dusk", PortKind::Control, "gate"},
         {"year", PortKind::Control, ""},
         {"scale", PortKind::Control, ""},
         {"tempo", PortKind::Control, ""}},
        {{"day", 5.0f, 600.0f, 60.0f, "s"},
         {"year", 4.0f, 64.0f, 16.0f, ""},
         {"latitude", 0.0f, 80.0f, 40.0f, "°"},
         {"hemisphere", 0.0f, 1.0f, 0.0f, ""}}) {}   // 0 = norte, 1 = sul

    std::string type() const override { return "SEASON"; }

    Panel panel() const override {
        Panel p;
        // Personalização (autor, 27/11/2026: "avance na personalização dos
        // módulos"; jacks embaixo): os dois
        // ciclos do sol, DAY e YEAR, em par de 12 mm; a latitude embaixo.
        p.hp = 10;
        p.add(Widget::Kind::Label, "SEASON", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "season", "", 2.5f, 6.0f, 45.80f);
        p.add(Widget::Kind::Knob, "DAY", "day", 9.24f, 27.0f, 12.0f);
        p.add(Widget::Kind::Knob, "YEAR", "year", 29.56f, 27.0f, 12.0f);
        p.add(Widget::Kind::Knob, "LAT", "latitude", 14.40f, 46.0f);
        p.add(Widget::Kind::Toggle, "HEMI", "hemisphere", 30.40f, 48.0f);
        p.add(Widget::Kind::Jack, "RST", "in:reset", 19.05f, 76.0f);
        p.add(Widget::Kind::Jack, "SPD", "in:speed_cv", 31.75f, 76.0f);
        p.add(Widget::Kind::Jack, "SCL", "out:scale", 12.70f, 93.0f);
        p.add(Widget::Kind::Jack, "TEMPO", "out:tempo", 25.40f, 93.0f);
        p.add(Widget::Kind::Jack, "SUN", "out:sun", 38.10f, 93.0f);
        p.add(Widget::Kind::Jack, "LIGHT", "out:light", 6.35f, 110.0f);
        p.add(Widget::Kind::Jack, "DAWN", "out:dawn", 19.05f, 110.0f);
        p.add(Widget::Kind::Jack, "DUSK", "out:dusk", 31.75f, 110.0f);
        p.add(Widget::Kind::Jack, "YEAR", "out:year", 44.45f, 110.0f);
        return p;
    }

    void prepare(float sampleRate, std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        clearTime();
    }

    // A altura do Sol (radianos) para a fase do dia (0 = meia-noite) e a do
    // ano (0 = equinócio de março), na latitude dada (graus; negativa = sul).
    static double altitude(double dayPhase, double yearPhase, double latitudeDeg) noexcept {
        const double tw = 6.283185307179586;
        const double decl = 23.44 * 0.017453292519943 * std::sin(tw * yearPhase);
        const double lat = latitudeDeg * 0.017453292519943;
        const double hour = tw * (dayPhase - 0.5);   // 0 ao meio-dia
        return std::asin(std::sin(lat) * std::sin(decl) + std::cos(lat) * std::cos(decl) * std::cos(hour));
    }
    // A fração do dia com o Sol acima do horizonte.
    static double daylight(double yearPhase, double latitudeDeg) noexcept {
        int up = 0;
        for (int k = 0; k < 720; ++k)
            if (altitude((k + 0.5) / 720.0, yearPhase, latitudeDeg) > 0.0) ++up;
        return up / 720.0;
    }
    double dayPhase() const noexcept { return t_ - std::floor(t_); }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 7) return;
        const AudioBlock* reset = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* scv = inputs.size() > 1 ? inputs[1] : nullptr;

        const double daySec = std::clamp(safe(parameterValue("day"), 60.0f), 5.0f, 600.0f);
        const double yearDays = std::round(std::clamp(safe(parameterValue("year"), 16.0f), 4.0f, 64.0f));
        const double latK = std::clamp(safe(parameterValue("latitude"), 40.0f), 0.0f, 80.0f);
        const bool south = safe(parameterValue("hemisphere"), 0.0f) >= 0.5f;
        const double lat = south ? -latK : latK;
        const int gateLen = std::max(1, static_cast<int>(0.05f * sr_));
        constexpr int kStep = 64;

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        for (std::size_t f = 0; f < frames; ++f) {
            if (reset) {
                const float r = safe(reset->at(0, f), 0.0f);
                if (prevReset_ < 0.5f && r >= 0.5f) clearTime();
                prevReset_ = r;
            }
            double speed = 1.0;
            if (scv) speed = std::exp2(std::clamp(static_cast<double>(safe(scv->at(0, f), 0.0f)), -4.0, 4.0));
            t_ += speed / (daySec * sr_);   // t em dias
            if (++counter_ >= kStep) {
                counter_ = 0;
                const double yp = (t_ / yearDays) - std::floor(t_ / yearDays);
                const double alt = altitude(dayPhase(), yp, lat);
                sun_ = static_cast<float>(std::max(0.0, std::sin(alt)));
                const bool up = alt > 0.0;
                if (up && !wasUp_) dawnLeft_ = gateLen;
                if (!up && wasUp_) duskLeft_ = gateLen;
                wasUp_ = up;
                year_ = static_cast<float>(yp);
                // a estação astronômica do lugar (no sul, o ano corre meio ciclo adiante)
                const double ys = south ? std::fmod(yp + 0.5, 1.0) : yp;
                static constexpr float kModes[4] = {5.0f, 1.0f, 3.0f, 2.0f};   // Lídio, Maior, Dórico, Eólio
                scale_ = kModes[static_cast<int>(ys * 4.0) % 4] / 11.0f;
                // o andamento: a luz (agora) e o calor (a estação: máximo no solstício de verão)
                const float warmth = static_cast<float>(0.5 + 0.5 * std::sin(6.283185307179586 * ys));
                tempo_ = std::clamp(0.15f + 0.45f * sun_ + 0.4f * warmth, 0.0f, 1.0f);
            }
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = sun_;
                outputs[1].at(c, f) = wasUp_ ? 1.0f : 0.0f;
                outputs[2].at(c, f) = dawnLeft_ > 0 ? 1.0f : 0.0f;
                outputs[3].at(c, f) = duskLeft_ > 0 ? 1.0f : 0.0f;
                outputs[4].at(c, f) = year_;
                outputs[5].at(c, f) = scale_;
                outputs[6].at(c, f) = tempo_;
            }
            if (dawnLeft_ > 0) --dawnLeft_;
            if (duskLeft_ > 0) --duskLeft_;
        }
    }

private:
    void clearTime() noexcept {
        t_ = 0.25;   // amanhece no equinócio de primavera (6 h)
        counter_ = kStepReset;
        prevReset_ = 0.0f;
        wasUp_ = false;
        dawnLeft_ = duskLeft_ = 0;
        sun_ = 0.0f; year_ = 0.0f; scale_ = 5.0f / 11.0f; tempo_ = 0.3f;
    }
    static constexpr int kStepReset = 63;
    static float safe(float v, float fallback) noexcept { return std::isfinite(v) ? v : fallback; }

    float sr_ = 48000.0f;
    double t_ = 0.25;
    int counter_ = 0, dawnLeft_ = 0, duskLeft_ = 0;
    float prevReset_ = 0.0f, sun_ = 0.0f, year_ = 0.0f, scale_ = 5.0f / 11.0f, tempo_ = 0.3f;
    bool wasUp_ = false;
};

} // namespace rasgo::modular
