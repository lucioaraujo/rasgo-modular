# 105 — FORM — a dinâmica da forma

Data: 21/11/2026. Estado: **implementação local**, aguardando a escuta do autor. Família MODULATE.

Proposta: `PROPOSTA_forma_e_andamento.md` §3; "pode implementar" (autor, 21/11/2026).
Código: `src/dsp/Form.hpp`.

- Caminhada pelos papéis INTRO · SUBIDA · CLÍMAX · QUEDA · CODA (coda
  absorvente); gramáticas ARCO (Freytag), ONDAS, CRESCIMENTO, CALMARIA,
  KISHŌ (a virada é corte seco, subito piano). LEN centra a duração.
- Ganho por camada: −DEPTH·(1 − tensão)·SEG·peso, ≤ 0 dB; pesos 0,6 / 0,85 /
  1 / 1,25 (o essencial desce menos, o enfeite mais); rampas de 20 ms.
- Gestos (GEST): respiração antes do clímax; subito no KISHŌ; morendo na coda.
- MODE: IMPOR / SEGUIR (ENV) / CONTRAPOR. CLK opcional: as trocas caem no
  pulso. LOOP recomeça depois de 2 s.
- Saídas: O1–O4, MIX, TENS, SECT, ROLE, CLMX, BRTH, END.
- Fora da seed e do VARIA (decisão da proposta).

Testes (`test_form_tempo_volta`): vale–clímax = DEPTH (11 ± 1 dB; medido
11,0), ganho ≤ 0 dB, sem degrau, END e LOOP, cada gramática termina, camada
essencial desce menos, determinismo.
