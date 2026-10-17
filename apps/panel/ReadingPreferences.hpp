#pragma once

// Preferência de leitura da UI, nunca parâmetro DSP nem estado do patch.
#include <algorithm>
#include <sstream>
#include <string>

namespace rasgo::panel {
struct ReadingPreferences {
    int size = 0; // altura JUCE em pixels lógicos: 12, 14 ou 16
    bool whiteText = false;
    int validSize() const { return std::clamp(size, 0, 2); }
    float fontHeight() const { return 12.0f + 2.0f * validSize(); }
    int width() const { return 186 + 40 * validSize(); }
    int height(int available) const {
        return std::clamp(232 + 40 * validSize(), 0, available > 120 ? available - 120 : 0);
    }
    std::string serialize() const {
        return "1 " + std::to_string(validSize()) + " " + (whiteText ? "1\n" : "0\n");
    }
    static ReadingPreferences parse(const std::string& text) {
        if (text.size() > 64) return {};
        std::istringstream input(text);
        int version = 0, size = 0, white = 0;
        std::string extra;
        if (!(input >> version >> size >> white) || (input >> extra)
            || version != 1 || size < 0 || size > 2 || white < 0 || white > 1)
            return {};
        return {size, white != 0};
    }
};
} // namespace rasgo::panel
