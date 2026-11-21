# 107 — VOLTA — o laço que volta

Data: 21/11/2026. Estado: **implementação local**, aguardando a escuta do autor. Família TIME.

Decisão do autor (21/11/2026): "turing reversível, módulo irmão, mas com um
nome mais curto". Do estudo da família (Navalha 2): transformação reversível.
Código: `src/dsp/Volta.hpp`.

- A base nunca é reescrita. Cada passo tem limiares fixos (do SEED): MUT
  troca o passo pelo valor alternativo quando o limiar dele está abaixo do
  knob; ERODE cala (pulso some, CV segura); MEM protege. Descer o knob
  devolve o laço exato. NEW (gatilho) é o único jeito de trocar a base.
- Saídas como as do TURING (CV, CV2, PULSE) e POS.

Testes: MUT transforma; voltar o MUT devolve a sequência idêntica; MEM 1
protege tudo; ERODE 1 cala todos os pulsos.
