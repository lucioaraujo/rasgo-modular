# 101 — GENDYN — a síntese estocástica de Xenakis

Data: 20/11/2026. Estado: **implementação local**, aguardando a escuta do autor.

Aprovado pelo autor (9/11/2026: "aprovo todos"); criado em 20/11/2026 ("crie todos os módulos novos pendentes").

Iannis Xenakis, síntese estocástica dinâmica (Formalized Music; GENDY3, 1991). Copiado e adaptado do Rasgo Synth Studio (`GendynOscillator.hpp`, mesmo autor), sem `std::random`. Desvios RASGO: FREQ/BAND (a altura presa com força 1 − BAND) e FRZ.

Código: `src/dsp/Gendyn.hpp`. Família SOURCE.

Testes (`test_gendyn`): com BAND 0 e FREQ 220, a altura medida é 220,2 Hz; sem FRZ, a onda muda de 50 períodos para outros (0,50 de diferença relativa); com FRZ, se repete (0,0001); com BAND e AMP no máximo, o pico fica em 0,61, finito.

## Na seed

Entra no rack da seed (família SOURCE), pela passada genérica. Um papel dedicado fica para depois da escuta.
