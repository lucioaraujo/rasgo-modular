// Como o app abre: a preferência ABRE, as variáveis de ambiente e a
// abertura segura depois de uma queda (apps/panel/StartupPolicy.hpp).

#include "panel/StartupPolicy.hpp"

#include <cstdio>
#include <cstdlib>

using namespace rasgo::panel;

static int falhas = 0;
#define CHECK(c) do { if (!(c)) { std::fprintf(stderr, "FALHOU %s:%d  %s\n", __FILE__, __LINE__, #c); ++falhas; } } while (0)

int main() {
    // padrão: sem preferência gravada, abre com seed novo
    CHECK(startModeFromCode("") == StartMode::seed);
    CHECK(startModeFromCode("lixo") == StartMode::seed);
    CHECK(planStartup({}).action == StartAction::newSeed);

    // ida e volta dos códigos e o ciclo do botão
    for (auto m : {StartMode::seed, StartMode::uncabled, StartMode::initPatch, StartMode::lastSession})
        CHECK(startModeFromCode(startModeCode(m)) == m);
    CHECK(nextStartMode(StartMode::seed) == StartMode::uncabled);
    CHECK(nextStartMode(StartMode::lastSession) == StartMode::seed);

    // cada preferência
    StartInputs in;
    in.pref = StartMode::uncabled;
    CHECK(planStartup(in).action == StartAction::newSeedUncabled);
    in.pref = StartMode::initPatch; in.initExists = true;
    CHECK(planStartup(in).action == StartAction::loadInit);
    in.initExists = false;  // sem patch inicial marcado: seed novo
    CHECK(planStartup(in).action == StartAction::newSeed);
    in.pref = StartMode::lastSession; in.lastExists = true;
    CHECK(planStartup(in).action == StartAction::loadLast);
    in.lastExists = false;
    CHECK(planStartup(in).action == StartAction::newSeed);

    // queda anterior: sem cabos, qualquer que seja a preferência
    for (auto m : {StartMode::seed, StartMode::uncabled, StartMode::initPatch, StartMode::lastSession}) {
        StartInputs c;
        c.pref = m; c.previousCrashed = true; c.initExists = true; c.lastExists = true;
        const auto p = planStartup(c);
        CHECK(p.action == StartAction::newSeedUncabled);
        CHECK(p.safeStart);
    }

    // variáveis de ambiente vêm antes de tudo (testes, render, CI)
    StartInputs e;
    e.envSeed = true; e.previousCrashed = true; e.pref = StartMode::lastSession;
    CHECK(planStartup(e).action == StartAction::fixedSeed);
    StartInputs r;
    r.envResume = true; r.lastExists = true; r.pref = StartMode::uncabled;
    CHECK(planStartup(r).action == StartAction::loadLast);
    r.lastExists = false;  // RASGO_RESUME sem sessão: segue a preferência
    CHECK(planStartup(r).action == StartAction::newSeedUncabled);

    if (falhas) { std::fprintf(stderr, "%d checagem(ns) falharam\n", falhas); return EXIT_FAILURE; }
    std::puts("startup policy: todas as checagens passaram");
    return EXIT_SUCCESS;
}
