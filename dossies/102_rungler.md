# 102 — RUNGLER — o coração do Benjolin

Data: 20/11/2026. Estado: **implementação local**, aguardando a escuta do autor.

Aprovado pelo autor (9/11/2026: "aprovo todos"); criado em 20/11/2026 ("crie todos os módulos novos pendentes").

Rob Hordijk, o Rungler do Benjolin e do Blippoo Box (conceito público dos textos dele). O Rasgo Synth Studio tem um Rungler (`Rungler.hpp`, mesmo autor); aqui refeito no desenho de Hordijk: três bits, o comparador PWM e a chave de laço. Desvio RASGO: LOOP por CV e um laço semeado.

Código: `src/dsp/Rungler.hpp`. Família SOURCE.

Testes (`test_rungler`): oito degraus (de 0 a 1); com LOOP 1, o padrão de 8 passos se repete para sempre; com LOOP 0, o bit de B entra e o padrão muda; em áudio, com MOD no máximo, finito e visitando os 8 degraus.

## Na seed

Entra no rack da seed (família SOURCE), pela passada genérica. Um papel dedicado fica para depois da escuta.
