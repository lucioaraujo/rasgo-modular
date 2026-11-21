# 106 — TEMPO — o andamento que respira

Data: 21/11/2026. Estado: **implementação local**, aguardando a escuta do autor. Família TIME.

Proposta: `PROPOSTA_forma_e_andamento.md` §4; "pode implementar" (autor, 21/11/2026).
Código: `src/dsp/Tempo.hpp`. Sai em RATE (V/oct de razão) para o jack RATE
novo do CLOCK (1 V = ×2).

- Camadas: salto-e-espera a cada SECT (±6 %); accelerando 1 + ACCEL·tensão;
  arco de frase (Todd 1992), média zero; rubato de Fourier (4 harmônicos,
  1/k², Asterismus), média zero; mola de 2ª ordem (ω 1,1·MOLA, ζ 0,62).
- Coda: ritardando do corredor que para (Friberg & Sundberg 1999), da razão
  atual até FINAL em RIT s, sem voltar; END no fim.
- Fermata: FERM segura o relógio (1/16) por FERMT tempos.
- MODE ESTRITO / ARCO / ELÁSTICO. ARCH sai como CV; BEAT é o pulso deformado.
- Fora da seed e do VARIA.

Testes: ritardando monotônico e chegada a FINAL ± 1 % (medido 0,600); média
da razão numa frase ≈ 1 com arco e rubato (medido 0,999); fermata segura e
volta.
