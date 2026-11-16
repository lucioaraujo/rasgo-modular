// Rasgo Modular — versão de entrega (streaming) da tomada do REC.
// Copyright (C) 2026 Lúcio de Araújo — AGPL-3.0-or-later.
//
// Master de entrega (16/11/2026, saida-ref 1.1; o autor: "nivele todos os
// instrumentos para que fiquem preparados para os spotifys da vida"). A
// tomada pós-safety, que o REC já tem na memória, vira <nome>.streaming.wav a
// -14 LUFS / <= -1 dBTP, com um <nome>.streaming.txt das medidas. No idioma do
// Modular: o mesmo `writeWav24` da tomada, sem dither (WavWriter.hpp explica
// por quê). Offline, num fio próprio; o som ao vivo e a tomada não mudam.
#pragma once

#include "io/WavWriter.hpp"
#include "saida/DeliveryMaster.h"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace rasgo::modular {

inline void writeStreamingVersion(const std::vector<float>& interleaved, const double sampleRate,
                                  const std::string& wavPath, const std::string& textPath,
                                  const std::string& takeName) {
    struct InterleavedSource final : saida::DeliveryMaster::Source {
        const std::vector<float>& v;
        std::size_t frame = 0;
        explicit InterleavedSource(const std::vector<float>& x) : v(x) {}
        bool rewind() override { frame = 0; return true; }
        std::size_t read(float* l, float* r, std::size_t frames) override {
            const std::size_t total = v.size() / 2;
            const std::size_t n = std::min(frames, total - frame);
            for (std::size_t i = 0; i < n; ++i) {
                l[i] = v[2 * (frame + i)];
                r[i] = v[2 * (frame + i) + 1];
            }
            frame += n;
            return n;
        }
    } source(interleaved);
    struct InterleavedSink final : saida::DeliveryMaster::Sink {
        std::vector<float> out;
        bool write(const float* l, const float* r, std::size_t frames) override {
            for (std::size_t i = 0; i < frames; ++i) {
                out.push_back(l[i]);
                out.push_back(r[i]);
            }
            return true;
        }
    } sink;
    sink.out.reserve(interleaved.size());
    const auto report = saida::DeliveryMaster::process(source, sampleRate, sink);
    if (report.failed || sink.out.empty()) return;
    if (!writeWav24(wavPath, sink.out, static_cast<std::uint32_t>(sampleRate), 2)) return;
    std::ofstream text(textPath);
    text << "Rasgo Modular - versao de entrega (streaming), saida-ref 1.1\n"
         << "tomada: " << takeName << " (post-safety)\n"
         << "alvo: -14 LUFS, true peak <= -1 dBTP; limitador no maximo 6 dB (guarda de dinamica)\n"
         << "entrada: " << report.inputLufs << " LUFS, " << report.inputTruePeakDb << " dBTP\n"
         << "ganho: " << report.gainDb << " dB; limitador: " << report.limiterReductionDb << " dB\n"
         << "saida: " << report.outputLufs << " LUFS, " << report.outputTruePeakDb << " dBTP\n"
         << (report.silent ? "silencio: copiada como veio\n"
             : report.reachedTarget ? "alvo atingido\n"
             : "alvo nao atingido sem esmagar a dinamica: entregue no mais alto possivel (os servicos sobem faixas baixas no modo normal)\n");
}

} // namespace rasgo::modular
