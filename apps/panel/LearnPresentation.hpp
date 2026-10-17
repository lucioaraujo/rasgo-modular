#pragma once

// Estado de apresentação do LEARN, independente do JUCE e do áudio.
// A leitura de parâmetros é feita pela UI, por try_lock; quando não há
// snapshot novo, conserva o último valor do MESMO controle, nunca do vizinho.
#include "panel/LearnEntry.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <locale>
#include <optional>
#include <sstream>
#include <string>

namespace rasgo::panel {

inline constexpr std::uint32_t kLearnTextPrimary = 0xfff0f3f8;
inline constexpr std::uint32_t kLearnTextSecondary = 0xffbec8d8;

inline std::string formatLearnValue(float value, const std::string& unit) {
    if (!std::isfinite(value)) return "--";
    // Não arredondar tempos pequenos (segundos) para zero. Locale fixa:
    // trocar a língua da UI não pode mudar a unidade nem gerar ambiguidade.
    std::ostringstream out;
    out.imbue(std::locale::classic());
    out << std::fixed << std::setprecision(std::fabs(value) < 0.01f ? 6 : 3)
        << value;
    auto text = out.str();
    while (!text.empty() && text.back() == '0') text.pop_back();
    if (!text.empty() && text.back() == '.') text.pop_back();
    if (text == "-0") text = "0";
    return text + (unit.empty() ? "" : " " + unit);
}

struct LearnPresentation {
    const LearnEntry* entry = nullptr;
    std::string key, title, value;
    int scroll = 0;
    int contentHeight = 0, viewHeight = 0;

    bool update(const LearnEntry* next, const std::string& nextKey,
                const std::string& nextTitle,
                const std::optional<std::string>& nextValue = std::nullopt) {
        const bool topicChanged = entry != next || key != nextKey;
        const auto reading = nextValue ? *nextValue : topicChanged ? "" : value;
        const bool changed = topicChanged || title != nextTitle || value != reading;
        if (!changed) return false;
        entry = next; key = nextKey; title = nextTitle; value = reading;
        if (topicChanged) scroll = 0;
        return true;
    }

    int maxScroll() const { return std::max(0, contentHeight - viewHeight); }
    void setGeometry(int content, int view) {
        contentHeight = std::max(0, content);
        viewHeight = std::max(0, view);
        scroll = std::clamp(scroll, 0, maxScroll());
    }
    void scrollBy(int delta) {
        // Converter antes da soma evita overflow mesmo com roda/tecla extrema.
        const auto requested = static_cast<long long>(scroll) + delta;
        scroll = static_cast<int>(std::clamp(requested, 0LL,
                                             static_cast<long long>(maxScroll())));
    }
};

} // namespace rasgo::panel
