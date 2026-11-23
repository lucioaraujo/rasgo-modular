#pragma once

// ============================================================================
// BancoEscalas — o banco de escalas do Trioio dentro do Rasgo Modular
// ============================================================================
//
// 2576 escalas com fonte citada (`third_party/trioio-escalas/`, cópia fixada;
// dados compactos em `BancoEscalasDados.inc`, gerados por
// `tools/gerar_banco_escalas.py`). Proposta: `dossies/PROPOSTA_banco_de_escalas.md`
// (opção A, 23/11/2026). Licença: AGPL-3.0-or-later (não entra na oferta
// dupla de `src/dsp`/`src/core`).
//
// Lido uma vez (a primeira chamada de `instancia()`, que o app faz no início,
// fora da thread de áudio). Depois é imutável: `procura(id)` é só leitura e
// pode ser chamada do áudio. Números lidos com o locale "C" (memória do
// projeto: `std::stof` segue o LC_NUMERIC e zerava parâmetros em pt_BR).
//
// `ligarAoQuantizer()` registra o banco no QUANTIZER (SCALE = BANCO).

#include "dsp/Quantizer.hpp"

#include <algorithm>
#include <cstdint>
#include <locale>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace rasgo::escalas {

struct Escala {
    std::uint32_t id = 0;          // FNV-1a do nome, 24 bits (o patch guarda isto)
    std::string nome;
    std::string familia;
    std::string fonte;
    float periodo = 1200.0f;       // cents
    std::vector<float> graus;      // cents desde a tônica, crescentes
};

class BancoEscalas {
public:
    static const BancoEscalas& instancia() {
        static const BancoEscalas b;
        return b;
    }

    const std::vector<Escala>& escalas() const noexcept { return escalas_; }
    // famílias na ordem do banco (a primeira vez que cada uma aparece)
    const std::vector<std::string>& familias() const noexcept { return familias_; }

    const Escala* procura(const std::uint32_t id) const noexcept {
        const auto it = porId_.find(id);
        return it == porId_.end() ? nullptr : &escalas_[it->second];
    }
    // o mesmo número que o gerador grava (para testes e para a interface)
    static std::uint32_t idDoNome(const std::string& nome) noexcept {
        std::uint32_t h = 0x811C9DC5u;
        for (const unsigned char c : nome) { h ^= c; h *= 0x01000193u; }
        return ((h >> 24) ^ h) & 0xFFFFFFu;
    }

    // registra o banco no QUANTIZER; idempotente
    static void ligarAoQuantizer() {
        instancia();
        rasgo::modular::Quantizer::setBankResolver(&resolve);
    }

private:
    BancoEscalas() {
        static const char* const kPedacos[] = {
#include "escalas/BancoEscalasDados.inc"
        };
        std::string texto;
        for (const char* p : kPedacos) texto += p;
        std::istringstream in(texto);
        std::string linha;
        while (std::getline(in, linha)) {
            if (linha.empty()) continue;
            std::vector<std::string> c;
            std::size_t a = 0;
            for (;;) {
                const std::size_t t = linha.find('\t', a);
                c.push_back(linha.substr(a, t == std::string::npos ? std::string::npos : t - a));
                if (t == std::string::npos) break;
                a = t + 1;
            }
            if (c.size() < 6) continue;
            Escala e;
            e.id = static_cast<std::uint32_t>(std::stoul(c[0]));
            e.nome = c[1];
            e.familia = c[2];
            e.fonte = c[5];
            e.periodo = num(c[3]);
            std::istringstream gs(c[4]);
            gs.imbue(std::locale::classic());
            float g = 0.0f;
            while (gs >> g) e.graus.push_back(g);
            if (e.graus.empty() || !(e.periodo > 1.0f)) continue;
            std::sort(e.graus.begin(), e.graus.end());
            if (std::find(familias_.begin(), familias_.end(), e.familia) == familias_.end())
                familias_.push_back(e.familia);
            porId_[e.id] = escalas_.size();
            escalas_.push_back(std::move(e));
        }
    }
    static float num(const std::string& s) {
        std::istringstream is(s);
        is.imbue(std::locale::classic());
        float v = 0.0f;
        is >> v;
        return v;
    }
    static bool resolve(const std::uint32_t id, rasgo::modular::Quantizer::BankScale& out) noexcept {
        const Escala* e = instancia().procura(id);
        if (e == nullptr) return false;
        out.cents = e->graus.data();
        out.count = static_cast<int>(e->graus.size());
        out.period = e->periodo;
        return true;
    }

    std::vector<Escala> escalas_;
    std::vector<std::string> familias_;
    std::unordered_map<std::uint32_t, std::size_t> porId_;
};

}  // namespace rasgo::escalas
