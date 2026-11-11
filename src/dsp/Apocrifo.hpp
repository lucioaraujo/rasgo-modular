#pragma once

#include "core/SignalGraph.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

// APOCRIFO (#77) — o patch cita o próprio passado (2026-11-11; dossiê
// `dossies/77_apocrifo.md`; estudo da família §4.8.3: "a tradução mais
// direta do nome RASGO: o patch rasga o próprio tempo").
//
// Escuta o áudio que passa (até 60 s de memória) e, por gatilho (CIT) ou
// por CHANCE, devolve um trecho de um passado distante — pelo menos IDADE
// segundos atrás. Três modos, conceitos do Rasgo Synth Studio (mesmo autor;
// conceito, não código):
//   0 CORROÍDA (`ApocryphalTear`): o trecho volta AO CONTRÁRIO e roído (cada
//     amostra segurada 3–9 vezes), e o presente se abaixa por baixo dele —
//     uma memória corrompida rasgando o tecido; bordas de 6 ms.
//   1 CORTE (`MontageEditCut`): um corte de montagem — o trecho volta limpo,
//     uma vez, e as duas emendas caem em cruzamentos de zero (a lâmina do
//     editor de fita). Desvio: 32 amostras (0,7 ms) de transição nas emendas,
//     porque no estéreo só um canal pode estar no zero.
//   2 PARÊNTESE (`ParenthesisGlitch`): um trecho do AGORA (DUR/4, 30–500 ms)
//     gagueja por DUR enquanto o patch segue por baixo; volta exatamente
//     onde o patch já está; bordas de 5 ms.
// A memória guarda a ENTRADA (nunca a própria saída: o apócrifo não cita a
// si mesmo). IDADE (CV, idade/60) diz de quão longe veio a citação.
// Memória alocada em prepare(); process() não aloca. Determinístico.
namespace rasgo::modular {

class Apocrifo final : public Signal {
public:
    static constexpr float kMemorySeconds = 60.0f;

    Apocrifo() : Signal(
        {{"in", PortKind::Audio, ""},
         {"trigger", PortKind::Control, "trig"}},
        {{"out", PortKind::Audio, ""},
         {"active", PortKind::Control, "gate"},
         {"age", PortKind::Control, ""}},
        {{"mode", 0.0f, 2.0f, 0.0f, ""},
         {"min_age", 5.0f, 55.0f, 20.0f, "s"},
         {"length", 0.1f, 3.0f, 0.8f, "s"},
         {"chance", 0.0f, 1.0f, 0.0f, ""},
         {"depth", 0.0f, 1.0f, 0.82f, ""},
         {"seed", 0.0f, 999.0f, 0.0f, ""}}) {}

    std::string type() const override { return "APOCRIFO"; }

    Panel panel() const override {
        Panel p;
        p.hp = 10;
        p.add(Widget::Kind::Label, "APOCRIFO", "", 2.5f, 2.0f);
        p.add(Widget::Kind::Display, "apocrifo", "", 2.5f, 6.0f, 45.8f);
        p.add(Widget::Kind::Knob, "MODO", "mode", 8.0f, 30.0f);
        p.add(Widget::Kind::Knob, "IDADE", "min_age", 23.0f, 30.0f);
        p.add(Widget::Kind::Knob, "DUR", "length", 38.0f, 30.0f);
        p.add(Widget::Kind::Knob, "CHANCE", "chance", 8.0f, 54.0f);
        p.add(Widget::Kind::Knob, "PROF", "depth", 23.0f, 54.0f);
        p.add(Widget::Kind::Knob, "SEED", "seed", 38.0f, 54.0f);
        p.add(Widget::Kind::Jack, "IN", "in:in", 7.0f, 96.0f);
        p.add(Widget::Kind::Jack, "CIT", "in:trigger", 19.0f, 96.0f);
        p.add(Widget::Kind::Jack, "OUT", "out:out", 7.0f, 116.0f);
        p.add(Widget::Kind::Jack, "ATIVO", "out:active", 19.0f, 116.0f);
        p.add(Widget::Kind::Jack, "IDADE", "out:age", 31.0f, 116.0f);
        return p;
    }

    void prepare(const float sampleRate, const std::size_t blockSize) override {
        Signal::prepare(sampleRate, blockSize);
        sr_ = std::isfinite(sampleRate) && sampleRate >= 8000.0f
            ? std::min(sampleRate, 384000.0f) : 48000.0f;
        const std::size_t n = std::min<std::size_t>(
            static_cast<std::size_t>(kMemorySeconds * sr_), std::size_t{1} << 23);
        for (auto& b : mem_) b.assign(n, 0.0f);
        size_ = n;
        write_ = 0;
        history_ = 0;
        state_ = State::Idle;
        prevTrig_ = 0.0f;
        prevLive_ = 0.0f;
        seedUsed_ = -1.0f;
        ageOut_ = 0.0f;
        fired_ = 0;
    }

    long citations() const noexcept { return fired_; }
    bool active() const noexcept { return state_ != State::Idle; }

    void process(const std::vector<const AudioBlock*>& inputs,
                 std::vector<AudioBlock>& outputs) noexcept override {
        if (outputs.size() < 3 || size_ == 0) return;
        const AudioBlock* in = inputs.size() > 0 ? inputs[0] : nullptr;
        const AudioBlock* trig = inputs.size() > 1 ? inputs[1] : nullptr;
        const int mode = static_cast<int>(std::lround(
            std::clamp(safe(parameterValue("mode"), 0.0f), 0.0f, 2.0f)));
        const float minAge = std::clamp(safe(parameterValue("min_age"), 20.0f), 5.0f, 55.0f);
        const float length = std::clamp(safe(parameterValue("length"), 0.8f), 0.1f, 3.0f);
        const float chance = std::clamp(safe(parameterValue("chance"), 0.0f), 0.0f, 1.0f);
        const float depth = std::clamp(safe(parameterValue("depth"), 0.82f), 0.0f, 1.0f);
        const float seed = std::round(std::clamp(safe(parameterValue("seed"), 0.0f), 0.0f, 999.0f));
        if (seed != seedUsed_) {
            seedUsed_ = seed;
            rng_ = 0xA0761D6478BD642FULL ^ (static_cast<std::uint64_t>(seed) * 0x9E3779B97F4A7C15ULL);
            if (rng_ == 0) rng_ = 1;
        }
        // CHANCE: até uma citação a cada ~10 s, em média
        const float pPerSample = chance * 0.1f / sr_;

        const std::size_t frames = outputs[0].frames();
        const std::size_t channels = outputs[0].channels();
        const std::size_t inCh = in ? std::min<std::size_t>(in->channels(), 2) : 0;
        for (std::size_t f = 0; f < frames; ++f) {
            float live[2] = {0.0f, 0.0f};
            if (in) {
                live[0] = safe(in->at(0, f), 0.0f);
                live[1] = inCh > 1 ? safe(in->at(1, f), 0.0f) : live[0];
            }
            // a memória guarda a ENTRADA
            mem_[0][write_] = live[0];
            mem_[1][write_] = live[1];

            bool want = false;
            if (trig) {
                const float t = safe(trig->at(0, f), 0.0f);
                if (prevTrig_ < 0.5f && t >= 0.5f) want = true;
                prevTrig_ = t;
            }
            if (!want && pPerSample > 0.0f && uniform() < pPerSample) want = true;
            if (want && state_ == State::Idle) arm(mode, minAge, length);

            float out[2] = {live[0], live[1]};
            render(live, out, depth);

            const float mid = 0.5f * (live[0] + live[1]);
            prevLive_ = mid;
            write_ = (write_ + 1) % size_;
            if (history_ < size_) ++history_;

            const float act = state_ != State::Idle ? 1.0f : 0.0f;
            for (std::size_t c = 0; c < channels; ++c) {
                outputs[0].at(c, f) = out[c < 2 ? c : 1];
                outputs[1].at(c, f) = act;
                outputs[2].at(c, f) = ageOut_;
            }
        }
    }

private:
    enum class State { Idle, Tear, WaitCut, PlayCut, WaitBack, Paren };
    static constexpr int kSplice = 32;     // CORTE: 0,7 ms de emenda
    static constexpr std::size_t kZeroSearch = 64;

    static float safe(const float v, const float fb) noexcept { return std::isfinite(v) ? v : fb; }
    float uniform() noexcept {
        rng_ ^= rng_ >> 12; rng_ ^= rng_ << 25; rng_ ^= rng_ >> 27;
        return static_cast<float>((rng_ * 0x2545F4914F6CDD1DULL) >> 40) / 16777216.0f;
    }
    // índice no anel de uma amostra `back` atrás de agora
    std::size_t at(const std::size_t back) const noexcept {
        return (write_ + size_ - (back % size_)) % size_;
    }
    float midAt(const std::size_t idx) const noexcept {
        return 0.5f * (mem_[0][idx] + mem_[1][idx]);
    }
    // cruzamento de zero mais próximo de `back` (em amostras atrás de agora)
    std::size_t nearestZero(const std::size_t back) const noexcept {
        std::size_t best = back, bestD = kZeroSearch + 1;
        const std::size_t lo = back > kZeroSearch + 1 ? back - kZeroSearch : 1;
        const std::size_t hi = std::min(back + kZeroSearch, history_ - 1);
        for (std::size_t b = lo; b < hi; ++b) {
            const bool s0 = midAt(at(b + 1)) >= 0.0f, s1 = midAt(at(b)) >= 0.0f;
            if (s0 != s1) {
                const std::size_t d = b > back ? b - back : back - b;
                if (d < bestD) { bestD = d; best = b; }
            }
        }
        return best;
    }

    void arm(const int mode, const float minAge, const float length) noexcept {
        const std::size_t len = std::max<std::size_t>(64, static_cast<std::size_t>(length * sr_));
        if (mode == 2) {   // PARÊNTESE: o agora
            const std::size_t loop = std::clamp<std::size_t>(len / 4,
                static_cast<std::size_t>(0.03f * sr_), static_cast<std::size_t>(0.5f * sr_));
            if (history_ < loop + 1) return;
            loopLen_ = loop;
            loopStartBack_ = loop;     // o trecho que acabou de passar
            loopPos_ = 0;
            total_ = len;
            left_ = len;
            edge_ = std::max<std::size_t>(1, static_cast<std::size_t>(0.005f * sr_));
            state_ = State::Paren;
            ageOut_ = 0.0f;
            ++fired_;
            return;
        }
        const std::size_t ageMin = static_cast<std::size_t>(minAge * sr_);
        if (history_ < ageMin + len + 2 * kZeroSearch + 2) return;   // ainda sem passado
        // começo sorteado entre o mais antigo e IDADE atrás
        const std::size_t oldest = history_ - 1 - kZeroSearch;
        const std::size_t span = oldest > ageMin + len ? oldest - ageMin - len : 0;
        std::size_t startBack = ageMin + len + static_cast<std::size_t>(uniform() * static_cast<float>(span));
        startBack = std::min(startBack, oldest);
        ageOut_ = std::clamp(static_cast<float>(startBack) / sr_ / kMemorySeconds, 0.0f, 1.0f);
        if (mode == 0) {   // CORROÍDA: ao contrário a partir do fim do trecho
            readBack_ = startBack - len;   // o fim do trecho; anda para trás no tempo
            total_ = len;
            left_ = len;
            hold_ = 3 + static_cast<int>(uniform() * 7.0f);
            holdCount_ = 0;
            edge_ = std::max<std::size_t>(1, static_cast<std::size_t>(0.006f * sr_));
            state_ = State::Tear;
        } else {           // CORTE: emendas em zero
            const std::size_t s = nearestZero(startBack);
            const std::size_t e = nearestZero(startBack > len ? startBack - len : 1);
            if (s <= e) return;
            readBack_ = s;
            left_ = s - e;
            fade_ = 0;
            state_ = State::WaitCut;
        }
        ++fired_;
    }

    void render(const float live[2], float out[2], const float depth) noexcept {
        const float mid = 0.5f * (live[0] + live[1]);
        const bool crossed = (mid >= 0.0f) != (prevLive_ >= 0.0f);
        switch (state_) {
        case State::Idle: return;
        case State::Tear: {
            const std::size_t elapsed = total_ - left_;
            // a leitura anda para TRÁS no tempo do trecho (ao contrário):
            // a cada amostra, uma amostra mais antiga — `readBack_` é a
            // distância desde AGORA, então cresce 2 por amostra (agora anda 1)
            if (holdCount_ == 0) {
                const std::size_t idx = at(readBack_ + 2 * elapsed);
                held_[0] = mem_[0][idx];
                held_[1] = mem_[1][idx];
            }
            holdCount_ = (holdCount_ + 1) % hold_;
            float env = 1.0f;
            if (elapsed < edge_) env = static_cast<float>(elapsed) / static_cast<float>(edge_);
            else if (left_ < edge_) env = static_cast<float>(left_) / static_cast<float>(edge_);
            for (int c = 0; c < 2; ++c)
                out[c] = live[c] * (1.0f - depth * env) + held_[c] * env;
            if (--left_ == 0) state_ = State::Idle;
            return;
        }
        case State::WaitCut:
            // enquanto espera o zero do presente, a origem recua com o tempo
            if (!crossed) { ++readBack_; return; }
            state_ = State::PlayCut;
            fade_ = 0;
            [[fallthrough]];
        case State::PlayCut: {
            // a fonte avança no tempo: a distância desde agora fica igual
            // (agora e a fonte andam juntos) — então `readBack_` é fixo
            const std::size_t idx = at(readBack_);
            const float a = fade_ < kSplice ? static_cast<float>(fade_) / kSplice : 1.0f;
            ++fade_;
            for (int c = 0; c < 2; ++c) out[c] = live[c] * (1.0f - a) + mem_[c][idx] * a;
            if (--left_ == 0) { state_ = State::WaitBack; fade_ = 0; lastSrc_[0] = mem_[0][idx]; lastSrc_[1] = mem_[1][idx]; }
            return;
        }
        case State::WaitBack: {
            // segura o silêncio do zero da fonte até o presente cruzar o zero
            if (!crossed && fade_ == 0) {
                out[0] = out[1] = 0.0f;
                return;
            }
            const float a = static_cast<float>(++fade_) / kSplice;
            for (int c = 0; c < 2; ++c) out[c] = live[c] * std::min(1.0f, a);
            if (fade_ >= kSplice) state_ = State::Idle;
            return;
        }
        case State::Paren: {
            const std::size_t elapsed = total_ - left_;
            // o trecho capturado no instante do parêntese: ele fica no passado
            // à distância loopStartBack_ + elapsed − loopPos_
            const std::size_t back = loopStartBack_ + elapsed - loopPos_;
            const std::size_t idx = at(back);
            loopPos_ = (loopPos_ + 1) % loopLen_;
            float env = 1.0f;
            if (elapsed < edge_) env = static_cast<float>(elapsed) / static_cast<float>(edge_);
            else if (left_ < edge_) env = static_cast<float>(left_) / static_cast<float>(edge_);
            const float w = env * depth;
            for (int c = 0; c < 2; ++c) out[c] = live[c] * (1.0f - w) + mem_[c][idx] * w;
            if (--left_ == 0) state_ = State::Idle;
            return;
        }
        }
    }

    float sr_ = 48000.0f;
    std::vector<float> mem_[2];
    std::size_t size_ = 0, write_ = 0, history_ = 0;
    State state_ = State::Idle;
    std::size_t readBack_ = 0, total_ = 0, left_ = 0, edge_ = 1;
    std::size_t loopLen_ = 1, loopStartBack_ = 0, loopPos_ = 0;
    int hold_ = 3, holdCount_ = 0, fade_ = 0;
    float held_[2] = {0.0f, 0.0f}, lastSrc_[2] = {0.0f, 0.0f};
    float prevTrig_ = 0.0f, prevLive_ = 0.0f, ageOut_ = 0.0f;
    std::uint64_t rng_ = 1;
    float seedUsed_ = -1.0f;
    long fired_ = 0;
};

}  // namespace rasgo::modular
