#pragma once

// Medição de loudness ITU-R BS.1770-4 / EBU R128 — momentary, short-term
// e integrated.
//
// Exigida por `RASGO_DOCUMENTATION/architecture/SAIDA_AUDIO_COMUM.md §3`:
// "medição BS.1770/LUFS momentary, short-term e integrated no diagnóstico
// e na renderização; loudness informa a decisão, mas não deve normalizar
// uma performance ao vivo automaticamente". A última parte é o motivo de
// isto ser um MEDIDOR e não um processador: ele não toca no sinal, em
// lugar nenhum. Quem decide o que fazer com o número é quem está ouvindo.
//
// ---- o que a norma manda ------------------------------------------------
//
// 1. Ponderação K: um shelf agudo (+4 dB) seguido de um passa-alta RLB.
//    Os coeficientes tabelados da norma são só pra 48 kHz; aqui eles são
//    DERIVADOS da taxa em uso pelos protótipos analógicos (a mesma
//    formulação da libebur128), senão medir a 44,1 ou 96 kHz daria um
//    número errado sem avisar.
// 2. Energia média quadrática por canal, somada com pesos por canal
//    (G = 1,0 pra esquerda e direita).
// 3. `L = -0,691 + 10·log10(Σ G_i · z_i)`.
// 4. Janelas: momentary = 400 ms, short-term = 3 s.
// 5. Integrated: blocos de 400 ms com 75% de sobreposição, com DUAS
//    portas — absoluta em −70 LUFS e relativa em −10 LU abaixo da média
//    dos blocos que passaram na primeira. A porta é o que impede o
//    silêncio entre frases de puxar a medida pra baixo.
//
// Framework-free: só `<cmath>`, `<cstddef>`, `<vector>`. Não aloca depois
// de `prepare()` — pode ser alimentado pelo thread de áudio.

#include <cmath>
#include <cstddef>
#include <vector>

namespace rasgo::modular {

// ---- true-peak (pico entre amostras) ------------------------------------
//
// O pico de amostra MENTE. Entre duas amostras o sinal reconstruído pelo
// conversor pode subir acima das duas, e um sinal que marca 0,0 dBFS na
// amostra chega a passar de +3 dBTP depois do DAC — ou depois de um
// codificador com perdas, que é o caso que interessa aqui: plataformas de
// streaming recodificam, e o que estoura é o true-peak, não o de amostra.
// Por isso o alvo de publicação tem teto em dBTP e não em dBFS.
//
// A BS.1770-4 (Anexo 2) pede sobreamostragem de pelo menos 4× antes de
// medir o pico. **Isto não é a tabela normativa de coeficientes**: é um
// interpolador polifásico de sinc janelado, projetado na taxa em uso. A
// escolha é deliberada — preferi um filtro que eu consigo derivar e
// verificar aqui a transcrever de memória uma tabela que eu não teria como
// conferir. O Anexo 2 define um filtro MÍNIMO; este é mais longo (16 taps
// por fase contra 12), então mede pelo menos tão bem. O teste compara
// contra o caso analítico conhecido — a senoide amostrada exatamente nos
// zeros do pico, onde o pico de amostra erra por ~3 dB.
class TruePeakMeter {
public:
    void prepare(const float sampleRate) {
        // A norma quer taxa efetiva de 192 kHz ou mais.
        const float sr = sampleRate > 0.0f ? sampleRate : 48000.0f;
        factor_ = sr <= 50000.0f ? 4 : (sr <= 100000.0f ? 2 : 1);
        design();
        hist_.assign(kTapsPerPhase * 2, 0.0);   // dois canais intercalados
        peak_ = 0.0;
    }

    void reset() {
        for (auto& v : hist_) v = 0.0;
        peak_ = 0.0;
    }

    void push(const float l, const float r) noexcept {
        // atraso comum aos dois canais, mais novo primeiro
        for (std::size_t k = kTapsPerPhase - 1; k > 0; --k) {
            hist_[k * 2]     = hist_[(k - 1) * 2];
            hist_[k * 2 + 1] = hist_[(k - 1) * 2 + 1];
        }
        hist_[0] = static_cast<double>(l);
        hist_[1] = static_cast<double>(r);

        for (int p = 0; p < factor_; ++p) {
            double al = 0.0, ar = 0.0;
            for (std::size_t k = 0; k < kTapsPerPhase; ++k) {
                const double h = coef_[static_cast<std::size_t>(p) * kTapsPerPhase + k];
                al += h * hist_[k * 2];
                ar += h * hist_[k * 2 + 1];
            }
            const double m = std::fabs(al) > std::fabs(ar) ? std::fabs(al)
                                                           : std::fabs(ar);
            if (m > peak_) peak_ = m;
        }
    }

    // dBTP. `-inf` vira −120 pra não poluir a leitura.
    float dBTP() const noexcept {
        if (peak_ <= 1e-12) return -120.0f;
        return static_cast<float>(20.0 * std::log10(peak_));
    }

    double linear() const noexcept { return peak_; }

private:
    static constexpr std::size_t kTapsPerPhase = 16;

    void design() {
        const std::size_t L = static_cast<std::size_t>(factor_);
        const std::size_t n = kTapsPerPhase * L;
        coef_.assign(kTapsPerPhase * 4, 0.0);
        if (L == 1) {                    // já está em 192k ou acima
            coef_[0] = 1.0;
            return;
        }
        std::vector<double> h(n, 0.0);
        const double center = (static_cast<double>(n) - 1.0) * 0.5;
        for (std::size_t i = 0; i < n; ++i) {
            const double m = static_cast<double>(i) - center;
            const double x = M_PI * m / static_cast<double>(L);
            const double s = (std::fabs(x) < 1e-12) ? 1.0 : std::sin(x) / x;
            const double t = static_cast<double>(i) / (static_cast<double>(n) - 1.0);
            const double w = 0.42 - 0.5 * std::cos(2.0 * M_PI * t)
                                  + 0.08 * std::cos(4.0 * M_PI * t);
            h[i] = s * w;
        }
        // Cada fase tem que ter ganho unitário em DC, senão a
        // sobreamostragem introduz uma ondulação que o medidor leria como
        // pico — mediria o filtro, não o sinal.
        for (std::size_t p = 0; p < L; ++p) {
            double sum = 0.0;
            for (std::size_t k = 0; k < kTapsPerPhase; ++k)
                sum += h[p + k * L];
            if (std::fabs(sum) < 1e-12) sum = 1.0;
            for (std::size_t k = 0; k < kTapsPerPhase; ++k)
                coef_[p * kTapsPerPhase + k] = h[p + k * L] / sum;
        }
    }

    int factor_ = 4;
    std::vector<double> coef_;
    std::vector<double> hist_;
    double peak_ = 0.0;
};

class LoudnessMeter {
public:
    // valor devolvido quando ainda não há material suficiente, ou quando
    // tudo que houve foi silêncio
    static constexpr float kSilence = -70.0f;

    // ---- alvo de publicação ---------------------------------------------
    //
    // Decisão do autor, 21 set. 2026: **streaming / plataformas**. Daí
    // saem os dois números abaixo, e é só por existir essa decisão que
    // eles fazem sentido — sem alvo declarado, escolher um perfil seria
    // arbitrário, e foi por isso que ficaram em aberto até agora.
    //
    // −14 LUFS é onde as plataformas normalizam; entregar mais alto não
    // soa mais alto, só é atenuado na reprodução, e a dinâmica que se
    // esmagou pra chegar lá não volta. −1 dBTP é a margem que evita
    // estouro quando o material é recodificado com perdas.
    //
    // Nada disto normaliza coisa alguma: o medidor não toca no sinal. São
    // números para a pessoa que está ouvindo decidir.
    static constexpr float kTargetLufs  = -14.0f;
    static constexpr float kTargetDbtp  =  -1.0f;

    void prepare(const float sampleRate) {
        sr_ = sampleRate > 0.0f ? sampleRate : 48000.0f;
        designKWeighting();
        tp_.prepare(sr_);

        // sub-blocos de 100 ms: 4 deles = momentary (400 ms), 30 =
        // short-term (3 s), e a sobreposição de 75% do integrated cai
        // naturalmente em passos de 100 ms
        subLen_ = static_cast<std::size_t>(std::lround(sr_ * 0.1f));
        if (subLen_ == 0) subLen_ = 1;

        hist_.assign(kShortTermSubs, 0.0);
        histCount_ = 0;
        histWrite_ = 0;
        blocks_.clear();
        blocks_.reserve(4096);          // ~27 min de tomada sem realocar
        subAcc_ = 0.0;
        subFill_ = 0;
        for (auto& s : state_) s = Biquad2State{};
    }

    void reset() { prepare(sr_); }

    // um par estéreo. `pushSamples` é `noexcept` e não aloca: pode vir do
    // thread de áudio.
    void push(const float l, const float r) noexcept {
        tp_.push(l, r);
        const double kl = filter(0, static_cast<double>(l));
        const double kr = filter(1, static_cast<double>(r));
        subAcc_ += kl * kl + kr * kr;
        if (++subFill_ < subLen_) return;

        const double meanSquare = subAcc_ / static_cast<double>(subLen_);
        subAcc_ = 0.0;
        subFill_ = 0;

        hist_[histWrite_] = meanSquare;
        histWrite_ = (histWrite_ + 1) % kShortTermSubs;
        if (histCount_ < kShortTermSubs) ++histCount_;

        // um bloco de 400 ms fecha a cada sub-bloco novo depois do 4º —
        // é exatamente a sobreposição de 75% que a norma pede
        if (histCount_ >= kMomentarySubs && blocks_.size() < blocks_.capacity())
            blocks_.push_back(windowMeanSquare(kMomentarySubs));
    }

    // LUFS das janelas móveis. `kSilence` enquanto não houver material.
    float momentary() const noexcept { return windowLufs(kMomentarySubs); }
    float shortTerm() const noexcept { return windowLufs(kShortTermSubs); }

    // Pico entre amostras da tomada inteira, em dBTP.
    float truePeakDbtp() const noexcept { return tp_.dBTP(); }

    // Quanto falta (LU) para o alvo. Positivo = está abaixo do alvo.
    float headroomToTargetLu() const noexcept {
        const float i = integrated();
        return (i <= kSilence) ? 0.0f : kTargetLufs - i;
    }

    // true se o true-peak já passou do teto do alvo — é a condição que
    // estoura na recodificação, e ela não aparece no pico de amostra.
    bool overTruePeakTarget() const noexcept {
        return truePeakDbtp() > kTargetDbtp;
    }

    // LUFS integrado com as duas portas da norma.
    float integrated() const noexcept {
        if (blocks_.empty()) return kSilence;

        // porta absoluta: −70 LUFS
        double sum = 0.0;
        std::size_t n = 0;
        for (const double z : blocks_)
            if (lufs(z) > -70.0f) { sum += z; ++n; }
        if (n == 0) return kSilence;

        // porta relativa: −10 LU abaixo da média dos que passaram
        const float relative = lufs(sum / static_cast<double>(n)) - 10.0f;
        sum = 0.0;
        n = 0;
        for (const double z : blocks_)
            if (lufs(z) > -70.0f && lufs(z) > relative) { sum += z; ++n; }
        if (n == 0) return kSilence;

        return lufs(sum / static_cast<double>(n));
    }

private:
    // 400 ms e 3 s em sub-blocos de 100 ms
    static constexpr std::size_t kMomentarySubs = 4;
    static constexpr std::size_t kShortTermSubs = 30;

    // `L = -0,691 + 10·log10(soma das energias ponderadas)`. O `z` que
    // chega aqui já é a soma dos dois canais (peso 1,0 em cada), então a
    // constante da norma se aplica direto.
    static float lufs(const double z) noexcept {
        if (z <= 0.0) return kSilence;
        const float v = static_cast<float>(-0.691 + 10.0 * std::log10(z));
        return v < kSilence ? kSilence : v;
    }

    double windowMeanSquare(const std::size_t subs) const noexcept {
        const std::size_t have = histCount_ < subs ? histCount_ : subs;
        if (have == 0) return 0.0;
        double sum = 0.0;
        for (std::size_t k = 0; k < have; ++k) {
            const std::size_t idx =
                (histWrite_ + kShortTermSubs - 1 - k) % kShortTermSubs;
            sum += hist_[idx];
        }
        return sum / static_cast<double>(have);
    }

    float windowLufs(const std::size_t subs) const noexcept {
        if (histCount_ == 0) return kSilence;
        return lufs(windowMeanSquare(subs));
    }

    // ---- ponderação K ---------------------------------------------------
    struct Biquad { double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0; };
    struct Biquad2State { double x1 = 0, x2 = 0, y1 = 0, y2 = 0,
                                 u1 = 0, u2 = 0, v1 = 0, v2 = 0; };

    // Protótipos analógicos da norma, bilinearizados na taxa em uso. Os
    // números mágicos são os da BS.1770-4 (e os mesmos da libebur128):
    // shelf em 1681,97 Hz com +3,9998 dB, passa-alta em 38,135 Hz.
    void designKWeighting() noexcept {
        {   // estágio 1 — shelf agudo
            const double f0 = 1681.974450955533;
            const double G  = 3.999843853973347;
            const double Q  = 0.7071752369554196;
            const double K  = std::tan(M_PI * f0 / static_cast<double>(sr_));
            const double Vh = std::pow(10.0, G / 20.0);
            const double Vb = std::pow(Vh, 0.4996667741545416);
            const double a0 = 1.0 + K / Q + K * K;
            shelf_.b0 = (Vh + Vb * K / Q + K * K) / a0;
            shelf_.b1 = 2.0 * (K * K - Vh) / a0;
            shelf_.b2 = (Vh - Vb * K / Q + K * K) / a0;
            shelf_.a1 = 2.0 * (K * K - 1.0) / a0;
            shelf_.a2 = (1.0 - K / Q + K * K) / a0;
        }
        {   // estágio 2 — passa-alta RLB
            const double f0 = 38.13547087602444;
            const double Q  = 0.5003270373238773;
            const double K  = std::tan(M_PI * f0 / static_cast<double>(sr_));
            const double den = 1.0 + K / Q + K * K;
            hp_.b0 = 1.0;
            hp_.b1 = -2.0;
            hp_.b2 = 1.0;
            hp_.a1 = 2.0 * (K * K - 1.0) / den;
            hp_.a2 = (1.0 - K / Q + K * K) / den;
        }
    }

    double filter(const int ch, const double x) noexcept {
        Biquad2State& s = state_[static_cast<std::size_t>(ch)];
        const double u = shelf_.b0 * x + shelf_.b1 * s.x1 + shelf_.b2 * s.x2
                       - shelf_.a1 * s.y1 - shelf_.a2 * s.y2;
        s.x2 = s.x1; s.x1 = x;
        s.y2 = s.y1; s.y1 = u;

        const double y = hp_.b0 * u + hp_.b1 * s.u1 + hp_.b2 * s.u2
                       - hp_.a1 * s.v1 - hp_.a2 * s.v2;
        s.u2 = s.u1; s.u1 = u;
        s.v2 = s.v1; s.v1 = y;
        return y;
    }

    float sr_ = 48000.0f;
    Biquad shelf_{}, hp_{};
    Biquad2State state_[2]{};

    std::size_t subLen_ = 4800;
    double subAcc_ = 0.0;
    std::size_t subFill_ = 0;

    std::vector<double> hist_;        // energias dos sub-blocos de 100 ms
    std::size_t histWrite_ = 0, histCount_ = 0;
    std::vector<double> blocks_;      // blocos de 400 ms p/ o integrated
    TruePeakMeter tp_;
};

}  // namespace rasgo::modular
