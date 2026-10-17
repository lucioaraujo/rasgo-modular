#include "panel/HeaderHelp.hpp"
#include "panel/ReadingPreferences.hpp"
#include <iostream>
#include <limits>
#include <set>

int main() {
    using namespace rasgo::panel;
    int failures = 0;
    const auto check = [&](bool good, const char* what) {
        if (!good) { ++failures; std::cerr << what << '\n'; }
    };
    std::set<HeaderAction> actions;
    std::set<std::string> keys;
    for (const auto& row : headerHelpRows()) {
        check(actions.insert(row.action).second, "duplicate command");
        check(keys.insert(row.key).second, "duplicate help identity");
        check(headerHelp(row.action) == &row, "stable help address");
        for (const auto lang : {Lang::en, Lang::pt, Lang::fr, Lang::es}) {
            const auto& entry = row.entries[static_cast<std::size_t>(langIndex(lang))];
            check(!tr(row.title, lang).empty(), "translated command title");
            check(entry.quick.size() >= 30 && entry.quick.size() < 1200, "bounded explanatory help");
            if (lang != Lang::en) check(entry.quick != row.entries[0].quick, "help translated, not English fallback");
        }
    }
    check(actions.size() == 22 && keys.size() == 22, "22 commands and editing fields covered");
    for (int i = 0; i < 22; ++i)
        check(headerHelp(static_cast<HeaderAction>(i)) != nullptr, "every action has help");
    check(headerHelp(static_cast<HeaderAction>(999)) == nullptr, "unknown action rejected");
    check(headerHelp(HeaderAction::bank)->entries[1].quick.find("Guarda") != std::string::npos,
          "BANK stores patches; it is not a file browser");
    check(headerHelp(HeaderAction::evolve)->entries[1].quick.find("imediato") != std::string::npos,
          "EVOLVE does not promise a timed transition");

    for (int size = 0; size <= 2; ++size)
        for (const bool white : {false, true}) {
            const ReadingPreferences pref{size, white};
            const auto restored = ReadingPreferences::parse(pref.serialize());
            check(restored.size == size && restored.whiteText == white, "reading round trip");
            check(pref.fontHeight() == 12 + 2 * size, "bounded reading font height");
            check(pref.width() == 186 + 40 * size, "column grows with text");
            for (int available : {0, 100, 120, 200, 660, 1080, std::numeric_limits<int>::min()})
                check(pref.height(available) >= 0 && pref.height(available) <= std::max(0, available),
                      "reading height bounded in small or invalid windows");
            check(pref.height(660) == 232 + 40 * size, "reading area grows with text");
        }
    for (const auto* text : {"", "1", "2 0 0", "1 -1 0", "1 3 0", "1 0 2",
                             "1 0 -1", "1 x 0", "1 0 0 trailing", "99999999999999999999 0 0"})
        check(ReadingPreferences::parse(text).serialize() == "1 0 0\n", "bad preferences reset safely");
    check(ReadingPreferences::parse(std::string(1000, 'x')).size == 0, "oversized preference rejected");
    check(ReadingPreferences{999, false}.fontHeight() == 16, "invalid size bounded");
    check(ReadingPreferences{-999, false}.fontHeight() == 12, "negative size bounded");
    if (failures) return 1;
    std::cout << "UI guidance: 22 commands, 4 languages; reading presets and persistence format OK\n";
}
