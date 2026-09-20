// Auditoria executável dos atalhos de teclado.
//
// Existe porque o teclado do app quebrou três vezes seguidas e as três
// correções foram feitas por leitura — eu relia o código, concluía que
// estava certo, e o autor voltava dizendo que não estava. O que faltava
// não era mais cuidado na leitura: era um teste que APERTA cada tecla.
//
// Cada `check` aqui é uma tecla de verdade sendo resolvida, em todas as
// formas em que o sistema pode entregá-la: minúscula, maiúscula (Shift
// ou CapsLock), só o código bruto sem caractere, e com Ctrl entregando
// caractere de controle em vez da letra. Foram exatamente essas as
// variações que quebraram — `n` funcionava com Shift e não sozinho;
// `Ctrl+Z` chegava como 26.

#include "ui/Shortcuts.hpp"
#include "panel/UiLanguage.hpp"

#include <cstdio>
#include <string>

using rasgo::ui::Shortcut;
using rasgo::ui::lookupShortcut;
using rasgo::ui::kShortcuts;

namespace {

int falhas = 0;

void check(const bool ok, const std::string& o_que) {
    if (!ok) { std::printf("  FALHOU: %s\n", o_que.c_str()); ++falhas; }
}

// O código bruto que o JUCE entrega para uma letra é a MAIÚSCULA ASCII.
constexpr int codigoBruto(const char letra) {
    return (letra >= 'a' && letra <= 'z') ? letra - 'a' + 'A' : letra;
}

const char* nome(const Shortcut s) {
    switch (s) {
    case Shortcut::none: return "nenhum";
    case Shortcut::seed: return "seed";
    case Shortcut::restore: return "repor";
    case Shortcut::vary: return "varia";
    case Shortcut::mutate: return "muda";
    case Shortcut::evolve: return "evolui";
    case Shortcut::cross: return "cruza";
    case Shortcut::uncable: return "descabear";
    case Shortcut::quit: return "sair";
    case Shortcut::undo: return "desfazer";
    case Shortcut::save: return "salvar";
    case Shortcut::bank: return "banco";
    case Shortcut::open: return "abrir";
    case Shortcut::rec: return "gravar";
    case Shortcut::zoomIn: return "zoom+";
    case Shortcut::zoomOut: return "zoom-";
    case Shortcut::zoomReset: return "zoom 100%";
    }
    return "?";
}

}  // namespace

int main() {
    // ---- 1. cada entrada resolve, em todas as formas de entrega -------
    //
    // Este é o "um a um" que o autor pediu: a tabela inteira, percorrida,
    // cada linha testada nas quatro formas.
    std::printf("atalhos declarados: %zu\n",
                sizeof(kShortcuts) / sizeof(kShortcuts[0]));

    for (const auto& e : kShortcuts) {
        const std::string rotulo =
            std::string(e.ctrl ? "Ctrl+" : "") + e.key + " -> " + nome(e.action);
        const int bruto = codigoBruto(e.key);
        const bool letra = (e.key >= 'a' && e.key <= 'z');

        // (a) a forma normal: o caractere minúsculo que a tecla produziu.
        check(lookupShortcut(bruto, e.key, e.ctrl) == e.action,
              rotulo + "  [minúscula]");

        // (b) com Shift ou CapsLock o caractere chega em MAIÚSCULA. Era
        //     este o caso invertido do bug do `n`: o autor observou que
        //     `Shift+N` acionava e `n` sozinho não.
        if (letra)
            check(lookupShortcut(bruto, bruto, e.ctrl) == e.action,
                  rotulo + "  [maiúscula / Shift / CapsLock]");

        // (c) teclado que não produz caractere nenhum (layout, IME,
        //     modificador engolindo o texto): sobra o código bruto.
        check(lookupShortcut(bruto, 0, e.ctrl) == e.action,
              rotulo + "  [sem caractere]");

        // (d) com Ctrl, o caractere pode vir como controle ASCII —
        //     Ctrl+Z = 26, Ctrl+S = 19. Foi assim que os `Ctrl+`
        //     morreram da primeira vez.
        if (e.ctrl && letra)
            check(lookupShortcut(bruto, e.key - 'a' + 1, true) == e.action,
                  rotulo + "  [caractere de controle]");
    }

    // ---- 2. o modificador separa de verdade ---------------------------
    //
    // `r` e `Ctrl+R` são ações DIFERENTES (repor o seed / gravar). Se a
    // tabela confundisse as duas, apertar `r` começaria a gravar — falha
    // cara e silenciosa.
    check(lookupShortcut('R', 'r', false) == Shortcut::restore,
          "r sozinho é REPOR");
    check(lookupShortcut('R', 'r', true) == Shortcut::rec,
          "Ctrl+R é GRAVAR");

    // Nenhum atalho sem Ctrl responde com Ctrl segurado, e vice-versa.
    for (const auto& e : kShortcuts) {
        const auto cruzado = lookupShortcut(codigoBruto(e.key), e.key, !e.ctrl);
        if (cruzado == Shortcut::none) continue;
        // Só é aceitável se houver OUTRA entrada declarando essa
        // combinação de propósito (o par r / Ctrl+R).
        bool declarado = false;
        for (const auto& o : kShortcuts)
            if (o.key == e.key && o.ctrl == !e.ctrl) declarado = true;
        check(declarado, std::string("tecla ") + e.key +
                         " vaza entre com e sem Ctrl");
    }

    // ---- 3. sem colisões ---------------------------------------------
    for (std::size_t i = 0; i < sizeof(kShortcuts)/sizeof(kShortcuts[0]); ++i)
        for (std::size_t j = i + 1; j < sizeof(kShortcuts)/sizeof(kShortcuts[0]); ++j)
            check(!(kShortcuts[i].key == kShortcuts[j].key
                    && kShortcuts[i].ctrl == kShortcuts[j].ctrl
                    && kShortcuts[i].action != kShortcuts[j].action),
                  std::string("duas ações para a mesma tecla: ") + kShortcuts[i].key);

    // ---- 4. teclas que NÃO são atalho ---------------------------------
    //
    // Digitar no rack não pode disparar nada por acidente.
    for (const char c : {'a', 'd', 'f', 'h', 'j', 'k', 'l', 'p', 'w', 'x'})
        check(lookupShortcut(codigoBruto(c), c, false) == Shortcut::none,
              std::string("'") + c + "' não deve ser atalho");

    // ---- 5. o tutorial não pode divergir do código --------------------
    //
    // O cartão TECLADO anunciava atalhos que ninguém conferia contra a
    // implementação. Aqui o documento passa a ser verificado: cada linha
    // da tabela tem que aparecer no cartão, nos quatro idiomas para os
    // `Ctrl+` (que são iguais em todos) e em português para o resto.
    using rasgo::panel::Lang;
    using rasgo::panel::tr;
    const std::string pt = tr(rasgo::panel::strings::tutKeysBody, Lang::pt);
    for (const auto& e : kShortcuts) {
        check(pt.find(e.tutorial) != std::string::npos,
              std::string("o tutorial não anuncia \"") + e.tutorial + "\"");
    }
    for (const Lang idioma : {Lang::en, Lang::pt, Lang::fr, Lang::es}) {
        const std::string t = tr(rasgo::panel::strings::tutKeysBody, idioma);
        for (const auto& e : kShortcuts)
            if (e.ctrl)
                check(t.find(e.tutorial) != std::string::npos,
                      std::string("idioma ") +
                      std::to_string(static_cast<int>(idioma)) +
                      " não anuncia " + e.tutorial);
    }

    if (falhas == 0) std::printf("atalhos: tudo certo\n");
    else             std::printf("atalhos: %d falha(s)\n", falhas);
    return falhas == 0 ? 0 : 1;
}
