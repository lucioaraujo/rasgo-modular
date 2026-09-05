#pragma once

// Entrada de áudio mínima via ALSA — contraparte de `AlsaSink.hpp` (captura
// em vez de reprodução). NÃO faz parte do `rasgo_modular_core`; alimenta o
// módulo `AUDIO-IN` (`src/dsp/AudioIn.hpp`) por `pushSamples()`, chamado de
// uma thread de captura dedicada em `panel_main.cpp` — nunca do thread de
// áudio que já toca o grafo (ver `AudioIn.hpp` pro porquê do desenho SPSC).

#include <alsa/asoundlib.h>

#include <cerrno>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace rasgo::panel {

class AlsaSource {
public:
    // `device`: nome ALSA/PCM ("default", "hw:1,0", um nome do PipeWire/
    // PulseAudio via `pipewire-alsa` etc.) — `RASGO_AUDIO_IN_DEVICE` no
    // ambiente escolhe, mesmo padrão do `RASGO_SEED`; sem a env var,
    // "default" (inalterado).
    AlsaSource(const unsigned rate, const unsigned channels,
               const snd_pcm_uframes_t period,
               const std::string& device = "default")
        : channels_(channels), period_(period), scratch_(period * channels) {
        if (snd_pcm_open(&pcm_, device.c_str(), SND_PCM_STREAM_CAPTURE, 0) < 0)
            throw std::runtime_error("ALSA: não abriu a captura '" + device + "'");

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
        unsigned periods = 8;  // mesmo fôlego do AlsaSink, mesma razão
        snd_pcm_hw_params_set_periods_near(pcm_, hw, &periods, nullptr);
        if (snd_pcm_hw_params(pcm_, hw) < 0)
            throw std::runtime_error("ALSA: parâmetros de captura recusados");
        actualRate_ = r;
        period_ = p;
        scratch_.assign(period_ * channels, 0);
        if (snd_pcm_prepare(pcm_) < 0)
            throw std::runtime_error("ALSA: captura não preparou");
    }

    ~AlsaSource() {
        if (pcm_) snd_pcm_close(pcm_);
    }

    // desbloqueia um `read()` pendente noutra thread (o `snd_pcm_readi`
    // bloqueante volta com erro) — usado no encerramento pra a thread de
    // captura não pendurar o `join()` se o PipeWire parou de entregar.
    void abort() noexcept {
        if (pcm_) snd_pcm_drop(pcm_);
    }

    AlsaSource(const AlsaSource&) = delete;
    AlsaSource& operator=(const AlsaSource&) = delete;

    unsigned rate() const noexcept { return actualRate_; }
    snd_pcm_uframes_t period() const noexcept { return period_; }

    // lê `period()` frames, devolve intercalado em [-1,1] em `interleavedOut`
    // (precisa de `period()*channels` amostras de espaço). Bloqueante — a
    // captura de verdade pacia sozinha no ritmo do hardware/PipeWire (ao
    // contrário da escrita, que precisou do relógio de parede manual do
    // `AlsaSink` nesta camada — a leitura não tem o mesmo problema porque
    // quem dita o ritmo aqui é a CHEGADA de amostras novas, não uma fila que
    // aceita tudo sem reagir). `false` = dispositivo não deu pra recuperar.
    bool read(float* interleavedOut) noexcept {
        snd_pcm_uframes_t done = 0;
        int tries = 0;
        while (done < period_ && tries++ < 8) {
            snd_pcm_sframes_t n = snd_pcm_readi(
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
        for (std::size_t i = 0; i < scratch_.size(); ++i)
            interleavedOut[i] = static_cast<float>(scratch_[i]) / 32768.0f;
        return true;
    }

private:
    snd_pcm_t* pcm_ = nullptr;
    unsigned channels_ = 2;
    unsigned actualRate_ = 48000;
    snd_pcm_uframes_t period_ = 256;
    std::vector<std::int16_t> scratch_;
};

}  // namespace rasgo::panel
