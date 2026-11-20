# 96 — TAPE — a fita magnética

Data: 20/11/2026. Estado: **implementação local**, aguardando a escuta do autor.

Aprovado pelo autor (9/11/2026: "aprovo todos"); criado em 20/11/2026 ("crie todos os módulos novos pendentes").

A máquina de fita da tape music (Schaeffer, Henry, Barron, Oram). Saturação adaptada do Rasgo Synth Studio (`TapeSaturator.hpp`, mesmo autor); conceito também no ChowTape (GPL), nenhum código copiado. Desvio RASGO: WEAR (dropouts e amassado) e DROP.

Código: `src/dsp/Tape.hpp`. Família TRANSFORM.

Testes (`test_tape`): DRIVE 1 multiplica o 3º harmônico de um seno de 200 Hz por ~45 sem estourar o nível; a 7,5 ips o 12 kHz sai mais baixo que a 30 ips (0,10 × 0,15 RMS); WOW e FLUT no máximo fazem o período variar 2 %; HISS 1 fica em −46 dBFS; WEAR 1 dá 9 dropouts em 20 s; finito.

## Na seed

Entra no rack da seed (família TRANSFORM), pela passada genérica. Um papel dedicado fica para depois da escuta.
