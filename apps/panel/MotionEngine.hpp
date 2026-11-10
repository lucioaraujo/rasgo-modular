#pragma once

#include "core/SignalGraph.hpp"
#include "panel/MotionField.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

// ============================================================================
// MotionEngine — camada de composição sobre o grafo
// ============================================================================
//
// Ver `dossies/ESTUDO_seed_composicao_generativa.md §3`.
//
// DOIS modos, coexistem:
//
//  1. `Binding` + `add()` — bindings MANUAIS (Walk/Oscillate/Attract),
//     um por vez, escolhidos à mão. Usado por `examples/peca_generativa_4`
//     e `tests/test_motion_engine`. É o protótipo §3.5/§3.6, mantido.
//
//  2. `inhabit()` + `MotionField` — a "MÃO CAÓTICA" (§3.7, 2026-09-07). A
//     engine varre o grafo e monta uma FIBRA por knob/slider/toggle (só
//     `MIXER`/`MASTER` ficam de fora). Um único campo caótico (Thomas)
//     move todas as fibras EM RELAÇÃO — coerente por construção, nunca
//     repete, nunca congela. A velocidade do campo segue a energia do
//     som (realimentação). "Gosto" = só a amplitude por fibra, de 4
//     pistas de palavra (`MotionField::motionReach`). De vez em quando
//     uma fibra "ousa" (excursão maior, alguns segundos).
//
// `rasgo_modular_core` é zero-dep e não sabe o que é "composição" — isto
// vive em `apps/panel/`, consumidor da API pública do grafo. Escreve por
// `SignalGraph::setParameterBase()` (aditivo — modulação por cabo já
// plugada continua somando por cima). `tick()` roda em taxa BAIXA (1×/
// frame de UI), nunca em `process()`. Determinístico: campo + fibras +
// RNG da ousadia semeados; mesmos `dt` → mesma trajetória.

namespace rasgo::panel {

class MotionEngine {
public:
    enum class Behavior { Walk, Oscillate, Attract };

    struct Binding {
        std::size_t node = 0;
        std::string paramId;
        Behavior behavior = Behavior::Walk;
        float lo = 0.0f, hi = 1.0f;
        float start = std::numeric_limits<float>::quiet_NaN();
        float rateHz = 0.05f;
        std::size_t nodeB = 0;
        std::string paramIdB;
        float gain = 0.3f;
        std::uint64_t seed = 0x9E3779B97F4A7C15ULL;
        float value = 0.0f;
        float target = 0.0f;
        float phase = 0.0f;
        std::uint64_t rng = 0;
    };

    void add(Binding b) noexcept {
        b.rng = b.seed;
        b.value = std::isnan(b.start) ? 0.5f * (b.lo + b.hi)
                                      : clampf(b.start, b.lo, b.hi);
        b.target = b.value;
        b.phase = 0.0f;
        bindings_.push_back(b);
    }

    void clear() noexcept { bindings_.clear(); fibers_.clear(); }
    std::size_t bindingCount() const noexcept { return bindings_.size(); }
    std::size_t fiberCount() const noexcept { return fibers_.size(); }

    // o músico mexeu na FIAÇÃO — re-lê quais params têm modulação entrando
    // (a fibra de um param recém-cabeado passa a mexer bem menos, pra não
    // brigar com o LFO/env que ele plugou). Não reseeda o campo nem mexe
    // no que já está respirando.
    void refreshCables(rasgo::modular::SignalGraph& graph) noexcept {
        for (auto& f : fibers_)
            f.cabled = graph.parameterIsModulated(f.node, f.paramId);
    }

    // ---- modo 2: a mão caótica ------------------------------------------
    struct Fiber {
        std::size_t node = 0;
        std::string paramId;
        int kind = 0;                 // 0 knob · 1 slider · 2 toggle
        MotionReach reach = MotionReach::Free;
        float wx = 0, wy = 0, wz = 0; // projeção do campo
        float center = 0, pmin = 0, pmax = 1;
        float amp = 0.1f;             // fração do range
        float tau = 1.0f;
        bool cabled = false;
        bool quantize = false;        // knob/slider "de contagem"
        float value = 0.0f;
        float boldMul = 1.0f;
        float toggleDwell = 0.0f;     // s desde o último flip
    };

    // varre `shown` e monta uma fibra por knob/slider/toggle (menos
    // MIXER/MASTER). `seed` mistura curSeed do painel — 0 = estável.
    void inhabit(rasgo::modular::SignalGraph& graph, const std::uint64_t seed,
                 const std::vector<std::size_t>& shown) {
        fibers_.clear();
        ++generation_;
        homeLearned_ = 0.0f; refAcc_ = 0.0f; slowDb_ = -60.0f; homePull_ = 0.0f;
        slowPow_ = 0.0f; relGatedFor_ = 0.0f;
        slowPrimed_ = false;
        field_.seed(seed ^ 0xA5A5C0FFEEULL);
        boldRng_ = seed ? (seed * 0x2545F4914F6CDD1DULL) ^ 0xD1B54A32D192ED03ULL
                        : 0xD1B54A32D192ED03ULL;
        boldCd_ = 6.0f + 14.0f * bu01();
        for (const std::size_t id : shown) {
            if (id >= graph.nodeCount()) continue;
            auto& node = graph.node(id);
            const std::string t = node.type();
            // MIXER e MASTER: o músico mistura, a engine não.
            // SIGNAL-IN também fica de fora, por decisão do autor em 25
            // set. 2026: os parâmetros dele (`gain`, `bend`, `cc_num`)
            // governam a ENTRADA de fora — microfone, instrumento, MIDI.
            // Mexer neles sozinho não é variar a música: é mexer no
            // aparelho de outra pessoa no meio da execução.
            // Analysis calibration is chosen by the musician. Moving LISTEN's
            // crossovers continuously would keep inhibiting its attack gates.
            if (t == "MIXER" || t == "MASTER" || t == "SIGNAL-IN" || t == "LISTEN") continue;
            for (const auto& w : node.panel().widgets) {
                int kind;
                switch (w.kind) {
                case rasgo::modular::Widget::Kind::Knob:   kind = 0; break;
                case rasgo::modular::Widget::Kind::Slider: kind = 1; break;
                case rasgo::modular::Widget::Kind::Toggle: kind = 2; break;
                default: continue;
                }
                if (w.bind.empty() || w.bind.rfind("in:", 0) == 0
                    || w.bind.rfind("out:", 0) == 0
                    || w.bind == "listen"    // ON do SIGNAL-IN: só à mão
                    || w.bind == "pit_keep" || w.bind == "pit_oct" // SCOPE: escolha de quem toca
                    || (t == "PHRASE" && w.bind == "seed")) // motif identity
                    continue;
                const rasgo::modular::ParameterDescriptor* d = nullptr;
                for (const auto& pr : node.parameters())
                    if (pr.descriptor.id == w.bind) { d = &pr.descriptor; break; }
                if (!d) continue;
                const float range = d->maximum - d->minimum;
                if (range <= 0.0f) continue;

                Fiber f;
                f.node = id;
                f.paramId = w.bind;
                f.kind = kind;
                f.reach = motionReach(w.bind);
                // Sliders de nota/fader mantêm a proteção histórica. No
                // ENVELOPE, mudar knob para slider é só apresentação: não
                // pode reduzir a excursão nem mudar a música de seeds antigas.
                const bool contour = t == "ENVELOPE" && (w.bind == "attack"
                    || w.bind == "decay" || w.bind == "sustain" || w.bind == "release");
                if (kind == 1 && f.reach == MotionReach::Free && !contour)
                    f.reach = MotionReach::Hot;
                f.amp = motionAmplitude(f.reach);
                f.tau = motionTau(f.reach);
                f.pmin = d->minimum;
                f.pmax = d->maximum;
                f.center = clampf(graph.parameterUserValue(id, w.bind),
                                  d->minimum, d->maximum);
                f.cabled = graph.parameterIsModulated(id, w.bind);
                f.quantize = (kind != 2)
                    && (f.reach == MotionReach::Structural)
                    && (range >= 3.0f);
                f.value = f.center;

                // projeção semeada por (seed, node, paramId)
                std::uint64_t h = 1469598103934665603ULL;
                for (const char c : t) { h ^= (unsigned char)c; h *= 1099511628211ULL; }
                for (const char c : w.bind) { h ^= (unsigned char)c; h *= 1099511628211ULL; }
                h ^= id + 0x9E3779B97F4A7C15ULL; h *= 1099511628211ULL;
                h ^= seed + 0xD1B54A32D192ED03ULL; h *= 1099511628211ULL;
                auto w01 = [&] {
                    h ^= h >> 13; h *= 0xBF58476D1CE4E5B9ULL; h ^= h >> 7;
                    return static_cast<float>((h >> 40)) / 16777216.0f;
                };
                f.wx = 2.0f * w01() - 1.0f;
                f.wy = 2.0f * w01() - 1.0f;
                f.wz = 2.0f * w01() - 1.0f;
                const float n = std::sqrt(f.wx*f.wx + f.wy*f.wy + f.wz*f.wz);
                if (n > 1e-6f) { f.wx /= n; f.wy /= n; f.wz /= n; }

                fibers_.push_back(std::move(f));
            }
        }
    }

    // Intensidade da mão: 0 congela (as fibras param no valor atual, sem
    // saltar), 1 é o comportamento histórico, 2 dobra a excursão pedida —
    // sempre sujeita ao teto de 0,48, que é o que impede a engine de
    // varrer o curso inteiro de um parâmetro.
    void setIntensity(const float v) noexcept {
        intensity_ = v < 0.0f ? 0.0f : (v > 2.0f ? 2.0f : v);
    }
    float intensity() const noexcept { return intensity_; }

    // Para a partitura (10/11/2026): o que a mão escreveu por último em cada
    // fibra. `fn(node, paramId, value, pmin, pmax)`.
    // muda a cada `inhabit` (seed nova, módulo novo…): as fibras são outras
    std::uint64_t generation() const noexcept { return generation_; }
    template <class Fn> void forEachFiber(Fn&& fn) const {
        for (const auto& f : fibers_) fn(f.node, f.paramId, f.value, f.pmin, f.pmax);
    }

    // CHÃO E TETO de volume da mão (10/11/2026, aprovado pelo autor). Numa
    // gravação do autor com o VARIA mais intenso, o patch andou 4 min de
    // −14 a −36 LUFS — a mão o levou a uma região alta e depois a uma
    // quase muda, e o equilíbrio de volume da seed não alcança isso (mede só
    // a abertura). Os CENTROS das fibras são a seed (equilibrada a −26 LUFS),
    // então voltar para perto deles devolve o volume — sem compressor, sem
    // tocar em MIXER/MASTER.
    //
    // Referência: com o LUFS momentâneo da saída (o app passa), o alvo
    // ABSOLUTO da seed (−26 LUFS, `setLoudnessTarget`) — medido na própria
    // tomada do autor: com o VARIA em 2, a seed sobe 6 LU nos primeiros
    // segundos, e uma referência aprendida ali já nasceria alta. Sem LUFS
    // (chamadores antigos, testes), cai no nível lento dos primeiros
    // segundos depois do `inhabit`. Nível lento: média com ~3 s.
    // Desvio além de 8 dB → a mão é puxada para casa, cheia em 14 dB; volta
    // a se soltar quando o nível retorna. O som segue variando; só não
    // passa minutos estourado ou sumido.
    static constexpr float kHomeLearnSeconds = 6.0f;
    static constexpr float kHomeSlowTau = 3.0f;
    // limiares medidos na tomada do autor (seed 12022678860901989809, VARIA
    // em 2): 8/14 dB deixava a região baixa (−33 LUFS, 7 LU abaixo) solta
    static constexpr float kHomeStartDb = 5.0f;
    static constexpr float kHomeFullDb = 10.0f;
    static constexpr float kHomeWarmupSeconds = 4.0f;   // o medidor ainda enche
    float homePull() const noexcept { return homePull_; }
    void setLoudnessTarget(const float lufs) noexcept { targetLufs_ = lufs; }
    float levelDeviationDb() const noexcept {
        return homeLearned_ >= kHomeLearnSeconds ? slowDb_ - refDb_ : 0.0f;
    }

    // avança `dt` s. `energy` (0..1) do som acelera a mão (realimentação).
    void tick(rasgo::modular::SignalGraph& graph, const float dt,
              const float energy = 0.5f,
              const float loudnessLufs = std::numeric_limits<float>::quiet_NaN()) noexcept {
        tickBindings(graph, dt);
        if (fibers_.empty()) return;

        // Intensidade ZERO congela de verdade: não escreve parâmetro
        // nenhum. Sem esta saída, a excursão ia a zero mas a primeira
        // passada ainda ESCREVIA o centro da fibra sobre o valor que o
        // patch tinha — um salto único, independente da amplitude. O
        // comentário de `setIntensity` prometia congelar; o teste mostrou
        // que não congelava. Quem gira o knob até o fim espera que a mão
        // largue o instrumento, não que ela o arrume antes de largar.
        if (intensity_ <= 0.0f) return;

        field_.step(dt, energy);

        // ousadia: de tempos em tempos uma fibra estica a excursão
        boldCd_ -= dt;
        if (boldCd_ <= 0.0f) {
            boldCd_ = 22.0f + 34.0f * bu01();
            if (!fibers_.empty()) {
                const std::size_t i = static_cast<std::size_t>(
                    bu01() * static_cast<float>(fibers_.size())) % fibers_.size();
                if (fibers_[i].reach == MotionReach::Free) {   // só os livres ousam
                    boldFiber_ = static_cast<long>(i);
                    boldLeft_ = 4.0f + 7.0f * bu01();
                }
            }
        }
        if (boldLeft_ > 0.0f) boldLeft_ -= dt;

        // realimentação protetora: som perto do teto → puxa tudo pro
        // centro (mais forte nos quentes/nível). Fecha o loop na direção
        // segura — o autor reportou clip com a engine ligada.
        const float duckAll = energy > 0.82f
            ? std::min(1.0f, (energy - 0.82f) * 3.5f) : 0.0f;

        // chão e teto (ver `homePull`): o mesmo puxão para o centro, nos
        // dois sentidos, guiado pelo nível LENTO contra o nível de casa
        {
            const bool absolute = std::isfinite(loudnessLufs);
            // Com LUFS: a leitura MOMENTÂNEA (400 ms) com a mesma lógica do
            // integrado da EBU R128 — porta absoluta em −70 e porta
            // RELATIVA 10 LU abaixo do nível atual, média em POTÊNCIA.
            // Num patch esparso (golpes e pausas) o curto prazo em dB lia
            // −32 a −50 com o integrado em −26 (seed 99999), e a mão
            // ficava presa em casa à toa.
            // a porta relativa vale para PAUSAS; uma queda que dura mais
            // de 4 s é o patch que ficou baixo de verdade (a tomada do autor
            // caiu 20 LU e ficou lá) — essas leituras passam a contar
            const bool relGated = absolute && slowPrimed_
                && loudnessLufs > -69.0f && loudnessLufs < slowDb_ - 10.0f;
            relGatedFor_ = relGated ? relGatedFor_ + dt : 0.0f;
            const bool silence = absolute
                && (loudnessLufs <= -69.0f || (relGated && relGatedFor_ < 4.0f));
            const float db = absolute
                ? loudnessLufs
                : 20.0f * std::log10(std::max(energy, 1.0e-4f));
            if (!silence) {
                if (absolute) {
                    const float p = std::pow(10.0f, db / 10.0f);
                    if (!slowPrimed_) { slowPow_ = p; slowPrimed_ = true; }
                    slowPow_ += (p - slowPow_) * (1.0f - std::exp(-dt / kHomeSlowTau));
                    slowDb_ = 10.0f * std::log10(std::max(slowPow_, 1.0e-12f));
                } else {
                    if (!slowPrimed_) { slowDb_ = db; slowPrimed_ = true; }
                    slowDb_ += (db - slowDb_) * (1.0f - std::exp(-dt / kHomeSlowTau));
                }
            }
            if (absolute && homeLearned_ < kHomeWarmupSeconds) {
                homeLearned_ += dt;   // o medidor ainda enche
            } else if (silence) {
                // pausa (ou bloco 10 LU abaixo): segura o nível lento
            } else if (absolute) {
                refDb_ = targetLufs_;
                homeLearned_ = std::max(homeLearned_, kHomeLearnSeconds);
                const float dev = std::fabs(slowDb_ - refDb_);
                const float goal = dev <= kHomeStartDb ? 0.0f
                    : std::min(1.0f, (dev - kHomeStartDb) / (kHomeFullDb - kHomeStartDb));
                homePull_ += (goal - homePull_) * (1.0f - std::exp(-dt / 1.5f));
            } else if (homeLearned_ < kHomeLearnSeconds) {
                refAcc_ += db * dt;
                homeLearned_ += dt;
                if (homeLearned_ >= kHomeLearnSeconds)
                    refDb_ = refAcc_ / homeLearned_;
            } else {
                const float dev = std::fabs(slowDb_ - refDb_);
                const float goal = dev <= kHomeStartDb ? 0.0f
                    : std::min(1.0f, (dev - kHomeStartDb) / (kHomeFullDb - kHomeStartDb));
                homePull_ += (goal - homePull_) * (1.0f - std::exp(-dt / 1.5f));
            }
        }

        const double fx = field_.nx(), fy = field_.ny(), fz = field_.nz();
        for (std::size_t i = 0; i < fibers_.size(); ++i) {
            Fiber& f = fibers_[i];
            const float range = f.pmax - f.pmin;

            // o MÚSICO mandou: se o valor de intenção do param mudou pra
            // longe do que a engine escreveu por último, é a mão dele —
            // re-ancora a fibra ali (a mão passa a respirar em torno do
            // novo valor, não volta pro anterior).
            const float userVal = graph.parameterUserValue(f.node, f.paramId);
            if (std::fabs(userVal - f.value)
                > 0.012f * std::max(1e-4f, range) + 1e-5f) {
                f.center = clampf(userVal, f.pmin, f.pmax);
                f.value = f.center;
                if (f.kind == 2) f.toggleDwell = 0.0f;
            }

            const float goalBold = (boldLeft_ > 0.0f
                                    && boldFiber_ == static_cast<long>(i))
                ? 2.0f : 1.0f;
            f.boldMul += (goalBold - f.boldMul) * (1.0f - std::exp(-dt / 2.5f));

            float raw = f.wx * static_cast<float>(fx)
                      + f.wy * static_cast<float>(fy)
                      + f.wz * static_cast<float>(fz);
            raw = std::tanh(raw);                       // [-1,1]

            // INTENSIDADE global da mão (knob VARIA).
            //
            // Pedido do autor na sessão de escuta de 23 set. 2026: "a
            // variação continua musical, algumas vezes as variações são
            // até discretas — o que acha de criarmos um knob para variar
            // o varia?". Multiplica a excursão de TODAS as fibras, antes
            // do teto de 0,48: em 1,0 é o comportamento que existia, e o
            // teto continua valendo, então subir a intensidade não leva a
            // engine a lugares que ela já não visitava.
            const float effAmp =
                std::min(0.48f, f.amp * f.boldMul * intensity_);
            const float cableK = f.cabled ? 0.45f : 1.0f;

            if (f.kind == 2) {
                // toggle: histerese + dwell mínimo (estrutural = dwell longo)
                f.toggleDwell += dt;
                float minDwell = (f.reach == MotionReach::Structural)
                    ? 25.0f : (f.reach == MotionReach::Hot ? 9.0f : 5.0f);
                // O dwell responde à INTENSIDADE, e o motivo vem do uso:
                // o autor relatou em 25 set. 2026 que "há botões que se
                // movem extremamente rápido". Medido, a frequência é
                // baixa (~2 trocas por minuto) — o que salta aos olhos é
                // que um toggle muda em UM quadro. Ele não pode varrer:
                // é binário.
                //
                // Como não há como suavizar a troca, o que se pode dar é
                // CONTROLE sobre ela. Abaixo de 1,0 o dwell cresce na
                // proporção inversa, então um VARIA suave deixa a
                // estrutura quieta e mexe só no que varre; acima de 1,0 o
                // dwell encurta, mas nunca abaixo da metade — trocar
                // `freeze` ou `hold` a cada segundo não é variação, é
                // outra música a cada segundo.
                minDwell /= std::max(0.5f, std::min(2.0f, intensity_));
                const float thr = 0.58f;
                // chão/teto: puxada forte, o toggle volta ao estado da seed
                // (um toggle não tem meio-caminho; e é ele que muda o
                // volume de uma vez — medido na tomada do autor)
                if (homePull_ > 0.5f) {
                    const float home = f.center >= 0.5f ? 1.0f : 0.0f;
                    if (f.value != home) { f.value = home; f.toggleDwell = 0.0f; }
                } else if (f.toggleDwell >= minDwell) {
                    if (raw > thr && f.value < 0.5f) {
                        f.value = 1.0f; f.toggleDwell = 0.0f;
                    } else if (raw < -thr && f.value >= 0.5f) {
                        f.value = 0.0f; f.toggleDwell = 0.0f;
                    }
                }
                graph.setParameterBase(f.node, f.paramId, f.value);
                continue;
            }

            // janela RELATIVA ao valor atual, limitada pela fração do
            // range — assim um param de range enorme e escala log
            // (freq 8–8000, cutoff 20–20000) respira em torno de onde
            // está, não varre milhares de Hz.
            const float relWin = std::max(std::fabs(f.center), 1e-3f) * 4.0f;
            const float win = std::min(range,
                std::min(effAmp * range, effAmp * relWin) * 1.4f + 1e-4f);
            float target = f.center + raw * (win / 1.4f) * cableK;
            target = clampf(target, f.center - win, f.center + win);
            target = clampf(target, f.pmin, f.pmax);
            // duck protetor: perto do clip, converge pro centro
            // o chão/teto puxa TODAS as fibras por igual: é a casa inteira
            // (a seed equilibrada) que devolve o volume, não só os quentes
            const float duck = std::max(duckAll
                * ((f.reach == MotionReach::Hot
                    || f.reach == MotionReach::Level) ? 1.0f : 0.55f), homePull_);
            target += (f.center - target) * duck;
            if (f.quantize) target = std::round(target);

            const float k = 1.0f - std::exp(-dt / std::max(0.05f, f.tau));
            f.value += (target - f.value) * k;
            f.value = clampf(f.value, f.pmin, f.pmax);
            graph.setParameterBase(f.node, f.paramId, f.value);
        }
    }

private:
    float intensity_ = 1.0f;   // knob VARIA — ver setIntensity
    // chão e teto de volume (ver `homePull`)
    float homeLearned_ = 0.0f, refAcc_ = 0.0f, refDb_ = -20.0f;
    float slowDb_ = -60.0f, homePull_ = 0.0f, slowPow_ = 0.0f, relGatedFor_ = 0.0f;
    bool slowPrimed_ = false;
    std::uint64_t generation_ = 0;
    float targetLufs_ = -26.0f;   // alvo da seed (`SeedBalance.hpp`)
    void tickBindings(rasgo::modular::SignalGraph& graph, const float dt) noexcept {
        for (auto& b : bindings_) {
            float v = b.value;
            switch (b.behavior) {
            case Behavior::Walk: {
                if (uniform(b.rng) < b.rateHz * dt)
                    b.target = b.lo + uniform(b.rng) * (b.hi - b.lo);
                const float coef = 1.0f - std::exp(-dt * b.rateHz * 3.0f);
                v += (b.target - v) * coef;
                break;
            }
            case Behavior::Oscillate: {
                b.phase += b.rateHz * dt;
                b.phase -= std::floor(b.phase);
                const float s = 0.5f + 0.5f * std::sin(6.2831853f * b.phase);
                v = b.lo + s * (b.hi - b.lo);
                break;
            }
            case Behavior::Attract: {
                const float t = clampf(
                    graph.parameterUserValue(b.nodeB, b.paramIdB), b.lo, b.hi);
                const float coef = 1.0f - std::exp(-dt * b.gain);
                v += (t - v) * coef;
                break;
            }
            }
            v = clampf(v, b.lo, b.hi);
            b.value = v;
            graph.setParameterBase(b.node, b.paramId, v);
        }
    }

    static float clampf(const float v, const float lo, const float hi) noexcept {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    static float uniform(std::uint64_t& s) noexcept {
        s ^= s >> 12; s ^= s << 25; s ^= s >> 27;
        const std::uint64_t x = s * 0x2545F4914F6CDD1DULL;
        return static_cast<float>(static_cast<std::uint32_t>(x >> 32))
            / 4294967296.0f;
    }
    float bu01() noexcept {
        boldRng_ ^= boldRng_ << 13; boldRng_ ^= boldRng_ >> 7;
        boldRng_ ^= boldRng_ << 17;
        return static_cast<float>(boldRng_ >> 40) / 16777216.0f;
    }

    std::vector<Binding> bindings_;
    std::vector<Fiber> fibers_;
    MotionField field_;
    std::uint64_t boldRng_ = 0xD1B54A32D192ED03ULL;
    float boldCd_ = 10.0f;
    float boldLeft_ = 0.0f;
    long boldFiber_ = -1;
};

}  // namespace rasgo::panel
