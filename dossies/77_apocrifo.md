# 77 — APÓCRIFO: o patch cita o próprio passado

Data: 11/11/2026. Estado: **em implementação** (estudo da família §4.8.3,
aprovado; "pode avançar nos módulos restantes"). Origem: `ApocryphalTear`,
`MontageEditCut` e `ParenthesisGlitch` do Rasgo Synth Studio (mesmo autor;
conceitos, não código).

## Portão conceitual

1. **Território.**
   - A citação e a montagem: o corte seco do cinema e o editor de fita, que
     corta no cruzamento de zero.
   - O parêntese da prosa: isola algo sem quebrar a frase.
   - A memória corrompida: o apócrifo, o texto do passado que volta
     alterado.
2. **Tensão.** O LOOPER e a MEMORY trabalham o passado **recente**. O
   APÓCRIFO traz o **distante** (≥ IDADE, de 5 a 55 s) como **ruptura**, não
   como textura. É a tradução mais direta do nome RASGO: o patch rasga o
   próprio tempo.
3. **Unidade.** A **memória da peça**: até 60 s do que passou.
4. **Percebe / conserva / transforma / recusa / esquece.**
   - Percebe o áudio de entrada.
   - Conserva 60 s, sempre da **entrada** (recusa citar a própria saída: o
     apócrifo não cita a si mesmo).
   - Transforma um trecho em citação, de três modos.
   - Esquece o que passou de 60 s.
5. **Regra, recurso, risco, limite.**
   - Regra: só cita o que tem pelo menos IDADE.
   - Recurso: os três modos.
   - Risco: o estalo. A resposta são bordas de 5–6 ms (corroída,
     parêntese) e emendas no cruzamento de zero (corte).
   - Limite: uma citação de cada vez.
6. **Consequência.** A forma ganha **retorno**: algo da abertura reaparece
   rasgando o meio da peça.
7. **Diferença no Modular.** LOOPER (laço do agora), MEMORY (grãos
   recentes), SAMPLER (amostra fixa): nenhum cita o passado distante como
   evento.
8. **Diferença para o Studio.**
   - Lá são três mecanismos internos do motor, sobre o render inteiro.
   - Aqui é um módulo com gatilho e chance visíveis, sobre o áudio que
     passa por ele.
   - No CORTE, uma emenda de 32 amostras (0,7 ms) ajuda o canal que não está
     no zero, já que no estéreo só um canal pode estar.
9. **Escuta.**
   - Êxito: reconhecer a abertura voltando aos 40 s, ao contrário ou num
     corte, e o presente retomando.
   - Fracasso: clique nas emendas, ou citações tão frequentes que viram
     textura.
   - Nominal: CHANCE 0 e nenhum gatilho.
10. **Proveniência.** Conceitos do Studio (mesmo autor). Código próprio.

## Comportamento

**Os três modos:**
- **CORROÍDA:** o trecho volta ao contrário, com sample-and-hold de 3–9
  amostras. O presente se abaixa em PROF. Bordas de 6 ms.
- **CORTE:** espera o zero do presente e toca o trecho limpo, com as
  emendas alinhadas ao zero da fonte (busca de ±64 amostras). No fim,
  silêncio até o presente cruzar o zero.
- **PARÊNTESE:** o trecho que acabou de passar (DUR/4, entre 30 e 500 ms)
  gagueja por DUR, com bordas de 5 ms.

**Disparo, saídas e memória:**
- O disparo vem pelo gatilho CIT, ou pela CHANCE (até uma citação a cada
  ~10 s em média), semeada por SEED.
- Saídas: OUT; ATIVO (gate); IDADE (de quão longe veio, /60 s).
- Memória: 2 × 60 s × fs, alocada em `prepare` (~23 MB a 48 kHz); teto de
  2²³ amostras por canal.

**Teste** (`test_apocrifo`), com a amplitude da entrada marcando o tempo:
- sem passado suficiente, não cita;
- o CORTE aos 35 s cita um trecho de 20,7 s atrás (pico citado 0,27 =
  amplitude de ~14 s), com o maior salto de 0,024, abaixo do limite de um
  seno de 200 Hz;
- a CORROÍDA abaixa o presente;
- o PARÊNTESE gagueja o agora;
- CHANCE 1 dá 7 citações em 60 s;
- a mesma SEED dá as mesmas citações.
