#pragma once

// Saída de áudio mínima via ALSA. NÃO faz parte do `rasgo_modular_core` -
// é do executável do painel de teste, como o `AudioOut` de qualquer
// front-end. Sem dependência no motor.

#include <alsa/asoundlib.h>

#include <cerrno>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace rasgo::panel {

class AlsaSink {
public:
    AlsaSink(const unsigned rate, const unsigned channels,
             const snd_pcm_uframes_t period)
        : channels_(channels), period_(period), scratch_(period * channels) {
        if (snd_pcm_open(&pcm_, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0)
            throw std::runtime_error("ALSA: não abriu o dispositivo 'default'");

        snd_pcm_hw_params_t* hw = nullptr;
        snd_pcm_hw_params_alloca(&hw);
        snd_pcm_hw_params_any(pcm_, hw);
        snd_pcm_hw_params_set_access(pcm_, hw, SND_PCM_ACCESS_RW_INTERLEAVED);
        snd_pcm_hw_params_set_format(pcm_, hw, SND_PCM_FORMAT_S16_LE);
        snd_pcm_hw_params_set_channels(pcm_, hw, channels);
        unsigned r = rate;
        snd_pcm_hw_params_set_rate_near(pcm_, hw, &r, nullptr);
        snd_pcm_uframes_t p = period;
        int dir = 0;
        snd_pcm_hw_params_set_period_size_near(pcm_, hw, &p, &dir);
        // buffer fundo de 16 períodos: folga contra xrun quando o thread de
        // desenho / re-prepare atrasa. (8 ainda recorria o "rachado" numa
        // máquina carregada; 16 ≈ 85 ms @ 256/48k — latência irrelevante
        // pra um painel de teste, e o jitter da UI cabe folgado.)
        unsigned periods = 16;
        snd_pcm_hw_params_set_periods_near(pcm_, hw, &periods, nullptr);
        if (snd_pcm_hw_params(pcm_, hw) < 0)
            throw std::runtime_error("ALSA: parâmetros de hardware recusados");
        actualRate_ = r;
        period_ = p;                 // o período REAL escolhido pelo ALSA
        scratch_.assign(period_ * channels, 0);
    }

    ~AlsaSink() {
        if (pcm_) {
            // `drop` (descarta o que resta na fila), NÃO `drain` (espera
            // tocar até o fim) — na saída do painel não há por que segurar o
            // processo esperando ~40 ms de buffer, e sob um PipeWire travado
            // o `drain` pode pendurar indefinidamente.
            snd_pcm_drop(pcm_);
            snd_pcm_close(pcm_);
        }
    }

    AlsaSink(const AlsaSink&) = delete;
    AlsaSink& operator=(const AlsaSink&) = delete;

    unsigned rate() const noexcept { return actualRate_; }
    snd_pcm_uframes_t period() const noexcept { return period_; }

    // `interleaved` tem period_*channels_ amostras em [-1, 1].
    // Devolve `false` quando o dispositivo não deu pra recuperar.
    //
    // RITMO POR RELÓGIO DE PAREDE: na camada ALSA do PipeWire o
    // `snd_pcm_writei` bloqueante NÃO segura o thread (ele aceita tudo e
    // descarta) — o thread produzia ~3,7x tempo real → xrun contínuo, que
    // soa como "tudo rachado". Aqui a gente mede o tempo decorrido e, se
    // está adiantado, dorme o excedente. Assim a produção fica em 1x
    // qualquer que seja o comportamento do `writei`, e o buffer de 8
    // períodos só precisa absorver o jitter.
    bool write(const float* interleaved) noexcept {
        for (std::size_t i = 0; i < scratch_.size(); ++i) {
            float v = interleaved[i];
            v = v > 1.0f ? 1.0f : (v < -1.0f ? -1.0f : v);
            scratch_[i] = static_cast<std::int16_t>(v * 32767.0f);
        }

        pace();

        snd_pcm_uframes_t done = 0;
        int tries = 0;
        while (done < period_ && tries++ < 8) {
            snd_pcm_sframes_t n = snd_pcm_writei(
                pcm_, scratch_.data() + done * channels_, period_ - done);
            if (n < 0) {
                if (n == -EAGAIN) {
                    std::this_thread::sleep_for(std::chrono::microseconds(500));
                    continue;
                }
                if (snd_pcm_recover(pcm_, static_cast<int>(n), 1) < 0)
                    return false;
                continue;
            }
            done += static_cast<snd_pcm_uframes_t>(n);
        }
        framesOut_ += period_;
        return true;
    }

private:
    using Clock = std::chrono::steady_clock;

    void pace() noexcept {
        const auto now = Clock::now();
        if (framesOut_ == 0) { start_ = now; return; }
        // quantos frames JÁ deviam ter saído a esta altura
        const double elapsed =
            std::chrono::duration<double>(now - start_).count();
        const double due = elapsed * actualRate_;
        const double ahead = static_cast<double>(framesOut_) - due;
        // só freia se estamos adiantados mais de um período (deixa a folga
        // do buffer trabalhar); nunca dorme mais que dois períodos
        const double slackFrames = static_cast<double>(period_);
        if (ahead > slackFrames) {
            double sleepFrames = ahead - slackFrames;
            const double maxSleep = 2.0 * period_;
            if (sleepFrames > maxSleep) sleepFrames = maxSleep;
            const auto ns = std::chrono::nanoseconds(
                static_cast<long long>(sleepFrames / actualRate_ * 1e9));
            std::this_thread::sleep_for(ns);
        }
    }

    snd_pcm_t* pcm_ = nullptr;
    unsigned channels_ = 2;
    unsigned actualRate_ = 48000;
    snd_pcm_uframes_t period_ = 256;
    std::vector<std::int16_t> scratch_;

    Clock::time_point start_{};
    std::uint64_t framesOut_ = 0;
};

}  // namespace rasgo::panel
