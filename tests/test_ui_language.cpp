// RASGO Modular — testes do vocabulário de UI do painel
// (`apps/panel/UiLanguage.hpp`). Não depende do `rasgo_modular_core`.

#include "panel/UiLanguage.hpp"

#include <cassert>
#include <cstdio>
#include <string>

using rasgo::panel::Lang;
using rasgo::panel::L4;
using rasgo::panel::tr;

static int failures = 0;
#define CHECK(cond)                                                            \
    do {                                                                      \
        if (!(cond)) {                                                        \
            std::printf("  FALHOU: %s (linha %d)\n", #cond, __LINE__);        \
            ++failures;                                                       \
        }                                                                    \
    } while (0)

static void testPicksTheRightLanguage() {
    const L4 s{"cut", "corte", "coupe", "corte"};
    CHECK(tr(s, Lang::en) == "cut");
    CHECK(tr(s, Lang::pt) == "corte");
    CHECK(tr(s, Lang::fr) == "coupe");
    CHECK(tr(s, Lang::es) == "corte");
}

static void testFallsBackToEnglishThenPortuguese() {
    // fr/es vazios -> caem no inglês
    const L4 partial{"only-en", "só-pt", "", ""};
    CHECK(tr(partial, Lang::fr) == "only-en");
    CHECK(tr(partial, Lang::es) == "only-en");
    // en também vazio (conteúdo em fase, ex.: LEARN) -> cai no português
    const L4 phased{"", "texto-base", "", ""};
    CHECK(tr(phased, Lang::en) == "texto-base");
    CHECK(tr(phased, Lang::fr) == "texto-base");
}

static void testCycleAndCodes() {
    CHECK(rasgo::panel::nextLang(Lang::en) == Lang::pt);
    CHECK(rasgo::panel::nextLang(Lang::pt) == Lang::fr);
    CHECK(rasgo::panel::nextLang(Lang::fr) == Lang::es);
    CHECK(rasgo::panel::nextLang(Lang::es) == Lang::en);

    for (const Lang l : {Lang::en, Lang::pt, Lang::fr, Lang::es})
        CHECK(rasgo::panel::langFromCode(rasgo::panel::langCode(l)) == l);

    CHECK(std::string(rasgo::panel::langCode(Lang::pt)) == "pt");
    CHECK(std::string(rasgo::panel::langLabel(Lang::fr)) == "FR");
    // código desconhecido -> inglês (o padrão)
    CHECK(rasgo::panel::langFromCode("xx") == Lang::en);
    CHECK(rasgo::panel::langFromCode("") == Lang::en);
}

static void testHeaderStringsAreComplete() {
    // todo texto do cabeçalho/tutorial/créditos tem as 4 línguas — o LEARN
    // é que fica em fase (en pode estar vazio lá, aqui não).
    namespace S = rasgo::panel::strings;
    const L4 must[] = {
        S::hdrVary, S::hdrStandby, S::hdrChange, S::hdrEvolve, S::hdrCross,
        S::hdrBank, S::hdrSave, S::hdrRec, S::hdrSeed, S::hdrTutorial,
        S::hdrAbout, S::close, S::rdModules, S::rdCables, S::rdPeak,
        S::tutTitle, S::tutSubtitle, S::tutCableTitle, S::tutCableBody,
        S::tutSeedTitle, S::tutSeedBody, S::tutVaryTitle, S::tutVaryBody,
        S::tutMoveTitle, S::tutMoveBody, S::tutZoomTitle, S::tutZoomBody,
        S::tutLearnTitle, S::tutLearnBody, S::aboutBody,
    };
    for (const auto& s : must) {
        CHECK(s.en && *s.en);
        CHECK(s.pt && *s.pt);
        CHECK(s.fr && *s.fr);
        CHECK(s.es && *s.es);
    }
}

int main() {
    testPicksTheRightLanguage();
    testFallsBackToEnglishThenPortuguese();
    testCycleAndCodes();
    testHeaderStringsAreComplete();
    if (failures == 0) {
        std::puts("test_ui_language: OK");
        return 0;
    }
    std::printf("test_ui_language: %d falha(s)\n", failures);
    return 1;
}
