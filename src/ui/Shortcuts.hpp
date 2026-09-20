#pragma once

// Tabela ÚNICA de atalhos de teclado, e a função pura que resolve uma
// tecla nela.
//
// ---- por que isto existe -------------------------------------------------
//
// O teclado do app JUCE quebrou três vezes seguidas, de três jeitos
// diferentes, e cada correção foi feita por dedução em cima da anterior:
//
//   1. o foco nunca chegava ao componente (a caixa de seed o retinha);
//   2. os `Ctrl+` eram comparados pelo código bruto de `getKeyCode()`,
//      que com modificador varia entre sistema e layout;
//   3. o `n` sozinho não acionava e o `Shift+N` sim — o mesmo problema da
//      (2), agora sem modificador nenhum.
//
// O padrão é claro: **a identidade de uma tecla não é confiável pelo
// código bruto**, e cada lugar que a decidia por conta própria decidia de
// um jeito. A correção não é mais um remendo: é ter UM lugar que decide e
// um teste que EXECUTA a decisão, em vez de eu reler o código e afirmar
// que está certo — foi exatamente a afirmação por leitura que falhou três
// vezes.
//
// O campo `tutorial` fecha o segundo furo: o cartão TECLADO do tutorial
// anunciava atalhos que ninguém verificava contra o código. O teste exige
// que cada linha desta tabela apareça no cartão, então atalho e documento
// não conseguem mais divergir em silêncio.
//
// Framework-free de propósito: o `keyPressed` do JUCE traduz o evento em
// `keyCode`/`textChar`/`ctrl` e pergunta aqui. Assim a tabela roda no
// `ctest`, sem janela nem dispositivo de áudio.

namespace rasgo::ui {

enum class Shortcut {
    none,
    // sem modificador
    seed, restore, vary, mutate, evolve, cross, uncable, quit,
    // com Ctrl (Cmd no macOS)
    undo, save, bank, open, rec, zoomIn, zoomOut, zoomReset,
};

struct ShortcutEntry {
    char key;             // a letra ou o símbolo, sempre em MINÚSCULA
    bool ctrl;            // exige a modificadora de comando?
    Shortcut action;
    const char* tutorial; // como o cartão TECLADO anuncia — o teste confere
};

// A tabela. A ordem não tem significado; o teste garante que não há duas
// entradas com a mesma combinação de tecla e modificador.
inline constexpr ShortcutEntry kShortcuts[] = {
    {'g', false, Shortcut::seed,      "g sorteia um seed"},
    {'v', false, Shortcut::vary,      "v liga/desliga a variação ao vivo"},
    {'m', false, Shortcut::mutate,    "m muda"},
    {'e', false, Shortcut::evolve,    "e evolui"},
    {'c', false, Shortcut::cross,     "c cruza"},
    {'n', false, Shortcut::uncable,   "n tira todos os cabos"},
    {'r', false, Shortcut::restore,   "r repõe o seed atual"},
    {'q', false, Shortcut::quit,      "q sai"},
    {'s', true,  Shortcut::save,      "Ctrl+S"},
    {'b', true,  Shortcut::bank,      "Ctrl+B"},
    {'r', true,  Shortcut::rec,       "Ctrl+R"},
    {'z', true,  Shortcut::undo,      "Ctrl+Z"},
    {'o', true,  Shortcut::open,      "Ctrl+O"},
    {'=', true,  Shortcut::zoomIn,    "Ctrl+="},
    {'+', true,  Shortcut::zoomIn,    "Ctrl+="},   // o mesmo tecla, com Shift
    {'-', true,  Shortcut::zoomOut,   "Ctrl+−"},
    {'0', true,  Shortcut::zoomReset, "Ctrl+0"},
};

inline constexpr char toLowerAscii(const int c) {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a')
                                  : static_cast<char>(c);
}

// Resolve uma tecla na tabela.
//
// `textChar` é o caractere que a tecla PRODUZIU (0 se não produziu
// nenhum); `keyCode` é o código bruto. Preferimos o caractere porque é o
// que o músico de fato digitou — com ou sem Shift, com ou sem CapsLock,
// em qualquer layout. O código bruto fica de reserva para as teclas que
// não geram caractere.
//
// Só trata atalhos de LETRA/SÍMBOLO. Teclas especiais (Esc, espaço,
// setas, Page Up/Down, Home/End) não passam por aqui: têm constante
// própria no JUCE, nunca foram ambíguas e não têm maiúscula.
inline Shortcut lookupShortcut(const int keyCode, const int textChar,
                               const bool ctrl) {
    // Com Ctrl segurado, `textChar` pode vir como caractere de controle
    // (Ctrl+Z = 26) em vez da letra. Nesse caso ele é inútil e o código
    // bruto é a fonte melhor — foi este caso exato que matou os `Ctrl+`.
    const bool textUsable = textChar >= 32;
    const char c = toLowerAscii(textUsable ? textChar : keyCode);
    for (const auto& e : kShortcuts)
        if (e.key == c && e.ctrl == ctrl) return e.action;
    return Shortcut::none;
}

}  // namespace rasgo::ui
