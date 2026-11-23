// O banco de escalas do Trioio no QUANTIZER (23/11/2026, opção A da
// `dossies/PROPOSTA_banco_de_escalas.md`): o banco inteiro lido, ids únicos e
// estáveis pelo nome, as alturas do BANCO saindo exatamente nos cents da
// escala, período não oitavante, e as 12 escalas de antes intocadas.

#include "escalas/BancoEscalas.hpp"

#include <clocale>
#include <cmath>
#include <iostream>
#include <set>
#include <vector>

using namespace rasgo::modular;
using rasgo::escalas::BancoEscalas;

namespace {
int g_failures = 0;
void check(const bool c, const char* const e) {
    if (!c) { std::cerr << "CHECK FALHOU: " << e << '\n'; ++g_failures; }
}
// varre a CV e devolve as alturas em cents desde a tônica (arredondado)
std::set<long> sweep(Quantizer& q) {
    q.prepare(48000.0f, 1);
    AudioBlock cv(48000.0f, 1, 1);
    std::vector<AudioBlock> out(3, AudioBlock(48000.0f, 1, 1));
    std::vector<const AudioBlock*> in(6, nullptr); in[0] = &cv;
    std::set<long> s;
    for (int i = -800; i <= 800; ++i) {
        cv.at(0, 0) = static_cast<float>(i) / 800.0f;
        q.process(in, out);
        s.insert(std::lround(out[0].at(0, 0) * 1200.0f));
    }
    return s;
}
}  // namespace

int main() {
    // os números lidos no locale "C" mesmo com o do sistema em vírgula
    std::setlocale(LC_ALL, "pt_BR.UTF-8");
    const auto& b = BancoEscalas::instancia();
    check(b.escalas().size() == 2576, "as 2576 escalas lidas");
    check(b.familias().size() >= 40, "as famílias do banco");
    std::set<std::uint32_t> ids;
    bool idsOk = true, grausOk = true;
    for (const auto& e : b.escalas()) {
        ids.insert(e.id);
        if (e.id == 0 || e.id != BancoEscalas::idDoNome(e.nome)) idsOk = false;
        if (e.graus.empty() || e.graus.size() > 128 || e.graus.front() != 0.0f) grausOk = false;
    }
    check(ids.size() == b.escalas().size(), "ids únicos");
    check(idsOk, "id = FNV do nome (o patch continua certo se o banco for reordenado)");
    check(grausOk, "graus de 1 a 128, começando na tônica");

    const auto* rast = b.procura(BancoEscalas::idDoNome("maqam Rast"));
    check(rast != nullptr && rast->graus.size() == 7 && std::fabs(rast->graus[2] - 350.0f) < 0.01f,
          "maqam Rast: 0 200 350 500 700 900 1050");

    BancoEscalas::ligarAoQuantizer();
    Quantizer q; q.setParameter("scale", 13.0f); q.setParameter("hysteresis", 0.0f);
    q.setParameter("range", 1.0f);
    q.setParameter("bank", static_cast<float>(rast->id));
    std::set<long> pc;
    for (long v : sweep(q)) pc.insert(((v % 1200) + 1200) % 1200);
    check(pc == std::set<long>({0, 200, 350, 500, 700, 900, 1050}), "o BANCO toca os cents do Rast (graus neutros)");

    // AFIN e MRPH não mexem numa escala do banco
    Quantizer q2 = q; q2.setParameter("tuning", 1.0f); q2.setParameter("morph", 0.7f);
    check(sweep(q2) == sweep(q), "AFIN e MRPH não valem no banco");

    // período não oitavante: as alturas repetem no período da escala
    const rasgo::escalas::Escala* nao = nullptr;
    for (const auto& e : b.escalas()) if (std::fabs(e.periodo - 1901.955f) < 0.01f) { nao = &e; break; }
    check(nao != nullptr, "há escala de período 1901,955 (terça justa + oitava)");
    if (nao) {
        Quantizer qp; qp.setParameter("scale", 13.0f); qp.setParameter("hysteresis", 0.0f);
        qp.setParameter("range", 3.0f); qp.setParameter("bank", static_cast<float>(nao->id));
        std::set<long> want;
        for (int k = -3; k <= 3; ++k) for (float g : nao->graus) want.insert(std::lround(k * nao->periodo + g));
        bool all = true;
        for (long v : sweep(qp)) if (!want.count(v)) all = false;
        check(all, "não oitavante: cada altura é grau + k·período");
    }

    // id desconhecido → cromática (nunca mudo)
    Quantizer qu; qu.setParameter("scale", 13.0f); qu.setParameter("hysteresis", 0.0f); qu.setParameter("bank", 12345.0f);
    Quantizer cr; cr.setParameter("scale", 0.0f); cr.setParameter("hysteresis", 0.0f);
    check(sweep(qu) == sweep(cr), "id desconhecido toca a cromática");

    // as 12 escalas e o crivo continuam iguais com o banco ligado e `bank` posto
    for (int s = 0; s <= 12; ++s) {
        Quantizer x; x.setParameter("scale", static_cast<float>(s)); x.setParameter("hysteresis", 0.0f);
        Quantizer y = x; y.setParameter("bank", static_cast<float>(rast->id));
        if (sweep(x) != sweep(y)) { check(false, "SCALE 0–12 ignoram o banco"); break; }
    }

    if (g_failures == 0) { std::cout << "RASGO Modular banco de escalas tests passed\n"; return 0; }
    std::cerr << g_failures << " verificacao(oes) falhou(aram)\n";
    return 1;
}
