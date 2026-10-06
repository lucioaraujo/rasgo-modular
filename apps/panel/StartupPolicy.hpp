#pragma once

// Como o Rasgo Modular ABRE — a regra, sem JUCE, para poder ser testada.
//
// Até a v0.1.4 o app abria sempre com um seed sorteado, tocando: posição de
// projeto ("não existe folha em branco silenciosa"). O retorno do fórum do
// VCV Rack (6 out. 2026) mostrou o limite disso: quem já tem os próprios
// patches quer começar deles ou de um rack sem cabos ("partir de um patch
// alheio não parece trabalho meu"), e um patch gerado na abertura é um risco
// se alguma coisa nele travar o app ("safety first"). Decisão do autor: a
// abertura vira uma PREFERÊNCIA, com o seed como padrão. O instrumento
// continua propondo patches; quem escolhe por onde começar é o músico.
//
// As quatro opções (botão ABRE, no cabeçalho):
//   seed          um patch novo, tocando (o padrão de sempre)
//   uncabled      todos os módulos, sem cabos — o "New" de um ambiente
//                 modular. Um seed é sorteado mesmo assim e fica como ponto
//                 de retorno (tecla r), exatamente como depois da tecla n.
//   initPatch     o patch que o músico marcou como inicial (init.rmp)
//   lastSession   a sessão como estava ao fechar (session.rmp)
//
// Segurança: se a sessão anterior não fechou normalmente (travou ou foi
// derrubada), a próxima abertura vem SEM CABOS, qualquer que seja a
// preferência — o patch que estava aberto pode ser a causa. Arquivo
// ausente ou ilegível cai no seed novo (o app decide isso ao carregar).

#include <string>

namespace rasgo::panel {

enum class StartMode { seed = 0, uncabled, initPatch, lastSession };

inline const char* startModeCode(StartMode m) noexcept {
    switch (m) {
        case StartMode::seed: return "seed";
        case StartMode::uncabled: return "uncabled";
        case StartMode::initPatch: return "init";
        case StartMode::lastSession: return "last";
    }
    return "seed";
}

inline StartMode startModeFromCode(const std::string& s) noexcept {
    if (s == "uncabled") return StartMode::uncabled;
    if (s == "init") return StartMode::initPatch;
    if (s == "last") return StartMode::lastSession;
    return StartMode::seed;  // vazio, desconhecido ou "seed"
}

inline StartMode nextStartMode(StartMode m) noexcept {
    return static_cast<StartMode>((static_cast<int>(m) + 1) % 4);
}

enum class StartAction {
    fixedSeed,       // RASGO_SEED=N (render determinístico, testes)
    newSeed,         // seed sorteado, tocando
    newSeedUncabled, // seed sorteado e cabos retirados
    loadInit,        // init.rmp
    loadLast,        // session.rmp
};

struct StartInputs {
    bool envSeed = false;        // RASGO_SEED definido
    bool envResume = false;      // RASGO_RESUME definido
    StartMode pref = StartMode::seed;
    bool previousCrashed = false;  // a sessão anterior não fechou normalmente
    bool initExists = false;
    bool lastExists = false;
};

struct StartPlan {
    StartAction action = StartAction::newSeed;
    bool safeStart = false;  // abriu sem cabos por causa de uma queda anterior
};

inline StartPlan planStartup(const StartInputs& in) noexcept {
    // As variáveis de ambiente continuam valendo antes de tudo: são o
    // caminho dos testes, do render determinístico e da CI.
    if (in.envSeed) return {StartAction::fixedSeed, false};
    if (in.envResume && in.lastExists) return {StartAction::loadLast, false};
    if (in.previousCrashed) return {StartAction::newSeedUncabled, true};
    switch (in.pref) {
        case StartMode::seed: return {StartAction::newSeed, false};
        case StartMode::uncabled: return {StartAction::newSeedUncabled, false};
        case StartMode::initPatch:
            return {in.initExists ? StartAction::loadInit : StartAction::newSeed, false};
        case StartMode::lastSession:
            return {in.lastExists ? StartAction::loadLast : StartAction::newSeed, false};
    }
    return {StartAction::newSeed, false};
}

} // namespace rasgo::panel
