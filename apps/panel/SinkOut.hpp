#pragma once

// `OUT` — o sink: o último nó de todo patch, o que os front-ends leem pra
// entregar ao dispositivo de áudio.
//
// Ele existe aqui, num header compartilhado, por duas razões. A primeira é
// que os dois front-ends precisam do MESMO nó (a `panelFactory` que
// desserializa um `.rmp` o fabrica pelo nome "OUT" — se os dois tivessem
// cópias divergentes, um patch salvo num soaria diferente no outro). A
// segunda é mais séria: **é aqui que mora a guarda de segurança**.
//
// ---- por que uma guarda no sink -----------------------------------------
//
// `RASGO_DOCUMENTATION/architecture/SAIDA_AUDIO_COMUM.md §3` exige
// "rejeição ou saneamento controlado de NaN/Inf, sem deixar um valor
// inválido ... atingir o dispositivo". O RASGO_MODULAR tem uma proteção de
// saída de excelência — `dsp/OutputStage.hpp`, com guarda de finitude,
// bloqueio de DC, limitador com look-ahead e teto de −1 dBFS — mas ela
// mora DENTRO do módulo MASTER. E o MASTER é um módulo como qualquer
// outro: o músico pode cabear qualquer coisa direto no OUT e passar por
// fora dele. Nesse caminho, até 2026-09-15, não havia proteção nenhuma:
//
//   - o painel X11 limitava a amplitude só na conversão pra int16 do ALSA,
//     e aquele clamp **não pega NaN** (`NaN > 1.0f` e `NaN < -1.0f` são
//     ambos falsos, então o NaN passa inteiro e o cast pra int16 é
//     comportamento indefinido — na prática, um estalo alto);
//   - o app JUCE não fazia clamp nenhum: float cru direto pro dispositivo.
//
// Um patch com realimentação mal resolvida, uma divisão por zero num
// módulo experimental ou um `.rmp` corrompido chegavam ao alto-falante em
// escala total. Isso é risco de equipamento e de audição, não estética.
//
// ---- o que esta guarda é, e o que NÃO é ---------------------------------
//
// É a camada de SEGURANÇA na acepção do documento comum: vem depois do
// master criativo e não pode ser desligada nem modulada. É de propósito
// mínima e transparente — saneia o não-finito e impõe o teto, e nada mais.
//
// NÃO é um segundo limitador: não tem look-ahead, não tem envelope, não
// colore. Quando o patch passa pelo MASTER (o caminho normal), o sinal já
// chega abaixo de −1 dBFS e **esta guarda nunca atua** — nenhuma amostra é
// alterada, nenhum timbre muda. Ela só existe pro caminho em que o músico
// escapou do MASTER, e aí o que ela faz é preferir um recorte duro a um
// estouro: feio, audível, e proposital — o recorte avisa que falta um
// MASTER no patch, enquanto silêncio ou distorção sutil esconderia.

#include "core/SignalGraph.hpp"

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace rasgo::panel {

class SinkOut final : public rasgo::modular::Signal {
public:
    // −1 dBFS, o mesmo teto do `OutputStage` (documento comum §3: "ponto de
    // partida conservador para exportação estéreo é ceiling de -1 dBTP")
    static constexpr float kCeiling = 0.891251f;

    SinkOut()
        : Signal({{"in", rasgo::modular::PortKind::Audio, ""}},
                 {{"out", rasgo::modular::PortKind::Audio, ""}}) {}

    std::string type() const override { return "OUT"; }

    void process(const std::vector<const rasgo::modular::AudioBlock*>& in,
                 std::vector<rasgo::modular::AudioBlock>& out) noexcept override {
        if (in[0] == nullptr) { out[0].clear(); return; }
        out[0].copyFrom(*in[0]);

        for (std::size_t ch = 0; ch < out[0].channels(); ++ch) {
            for (std::size_t i = 0; i < out[0].frames(); ++i) {
                const float v = out[0].at(ch, i);
                // `std::isfinite` é a ordem certa: testar o NaN ANTES de
                // comparar, porque toda comparação com NaN é falsa e um
                // clamp escrito como `v > hi ? hi : (v < lo ? lo : v)`
                // devolve o próprio NaN.
                if (!std::isfinite(v)) {
                    out[0].at(ch, i) = 0.0f;
                    ++nonFinite_;
                    continue;
                }
                if (v > kCeiling)       { out[0].at(ch, i) =  kCeiling; ++clipped_; }
                else if (v < -kCeiling) { out[0].at(ch, i) = -kCeiling; ++clipped_; }
            }
        }
    }

    // Telemetria: quantas amostras a guarda teve que consertar. Zero é o
    // normal — qualquer número diferente disso significa que o patch está
    // chegando ao dispositivo sem passar por um MASTER saudável.
    std::uint64_t nonFiniteCount() const noexcept { return nonFinite_; }
    std::uint64_t clippedCount() const noexcept { return clipped_; }
    void clearTelemetry() noexcept { nonFinite_ = 0; clipped_ = 0; }

private:
    std::uint64_t nonFinite_ = 0;
    std::uint64_t clipped_ = 0;
};

}  // namespace rasgo::panel
