#include "panel/LearnPresentation.hpp"

#include <cstdio>
#include <limits>

using namespace rasgo::panel;
namespace {
int failures = 0;
#define CHECK(c) do { if (!(c)) { std::fprintf(stderr, "FAIL %d: %s\n", __LINE__, #c); ++failures; } } while (0)

double luminance(std::uint32_t colour) {
    const auto linear = [](std::uint32_t channel) {
        const double v = channel / 255.0;
        return v <= 0.04045 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4);
    };
    return 0.2126 * linear((colour >> 16) & 255)
         + 0.7152 * linear((colour >> 8) & 255) + 0.0722 * linear(colour & 255);
}
}

int main() {
    CHECK(formatLearnValue(440, "Hz") == "440 Hz");
    CHECK(formatLearnValue(0.0001f, "s") == "0.0001 s");
    CHECK(formatLearnValue(-0.0f, "") == "0");
    CHECK(formatLearnValue(-12.5f, "dB") == "-12.5 dB");
    CHECK(formatLearnValue(65535, "") == "65535");
    CHECK(formatLearnValue(0.34567f, "") == "0.346");
    CHECK(formatLearnValue(std::numeric_limits<float>::infinity(), "Hz") == "--");
    CHECK(formatLearnValue(std::numeric_limits<float>::quiet_NaN(), "") == "--");

    LearnEntry a{"primeira frase", "explicação longa", "experimente"};
    LearnEntry b{"autre langue", "explanation", "try"};
    LearnPresentation p;
    CHECK(p.update(&a, "1|freq", "OSC FREQ", "BASE 440 Hz\nDSP 441 Hz"));
    p.setGeometry(500, 100); p.scrollBy(120);
    CHECK(p.scroll == 120 && p.maxScroll() == 400);
    CHECK(!p.update(&a, "1|freq", "OSC FREQ", "BASE 440 Hz\nDSP 441 Hz"));
    CHECK(p.update(&a, "1|freq", "OSC FREQ", "BASE 450 Hz\nDSP 451 Hz"));
    CHECK(p.scroll == 120); // leitura viva não volta ao topo da explicação
    CHECK(!p.update(&a, "1|freq", "OSC FREQ")); // try_lock sem amostra nova
    CHECK(p.value == "BASE 450 Hz\nDSP 451 Hz");
    CHECK(p.update(&a, "1|freq", "OSC FREQ (novo título)"));
    CHECK(p.scroll == 120); // mesmo verbete pode atualizar título
    CHECK(p.update(&a, "2|freq", "OSC FREQ")); // segunda instância
    CHECK(p.scroll == 0 && p.value.empty()); // nunca herdar o valor do vizinho
    p.scrollBy(90);
    CHECK(p.update(&b, "2|freq", "OSC FREQ")); // mudar idioma
    CHECK(p.scroll == 0);
    CHECK(p.update(&a, "jack", "OUT", ""));
    CHECK(p.value.empty());

    for (int view : {1, 50, 150, 500, 900}) {
        p.setGeometry(500, view);
        p.scrollBy(std::numeric_limits<int>::max());
        CHECK(p.scroll == std::max(0, 500 - view)); // fim sempre alcançável
        p.scrollBy(std::numeric_limits<int>::max());
        CHECK(p.scroll == p.maxScroll()); // sem overflow
        p.scrollBy(std::numeric_limits<int>::min());
        CHECK(p.scroll == 0);
    }
    p.setGeometry(500, 100); p.scrollBy(400);
    p.setGeometry(500, 450); CHECK(p.scroll == 50); // redimensionar
    p.setGeometry(-5, -10); CHECK(p.scroll == 0 && p.maxScroll() == 0);
    CHECK(p.update(nullptr, "", ""));
    CHECK(p.entry == nullptr && p.value.empty() && p.scroll == 0);

    // Contraste calculado sobre as superfícies opacas reais do app JUCE.
    // Não certifica o tamanho das letras, a percepção ou a tela do usuário.
    for (std::uint32_t bg : {0xff131a1aU, 0xff262b36U, 0xff0e1015U})
        for (auto fg : {kLearnTextPrimary, kLearnTextSecondary}) {
            const double ratio = (luminance(fg) + 0.05) / (luminance(bg) + 0.05);
            CHECK(ratio >= 7.0);
            std::printf("text #%06x on #%06x: %.2f:1\n", fg & 0xffffff, bg & 0xffffff, ratio);
        }
    std::printf("learn presentation: %s\n", failures ? "FAILED" : "OK");
    return failures ? 1 : 0;
}
