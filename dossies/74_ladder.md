# 74 — LADDER (ESCADA): o filtro do grave

Data: 11/11/2026. Estado: **em implementação** (fila aprovada pelo autor:
"aprovo todas… sequencia normal"; "avance"). Origem:
`PESQUISA_cardinal_baixo_e_osciladores.md` §3 e §8.1 ("a maior lacuna do
Modular no grave é o ladder") e `AUDITORIA_modulos_e_cardinal.md` (FILTER:
lacuna, sem ladder).

## Portão conceitual

1. **Território.** O filtro em escada de transistores de R. Moog (patente
   de 1969): quatro polos de 6 dB em cascata, e a ressonância como
   realimentação negativa da saída para a entrada. A escada de **diodos**
   (EMS, Roland TB-303) tem os degraus acoplados entre si: cada estágio
   carrega o vizinho, os polos se espalham, a ressonância fica mais
   "ácida" e menos redonda. Discretização sem atraso no laço (*zero-delay
   feedback*) por integração trapezoidal: V. Zavalishin, *The Art of VA
   Filter Design* (2012–2018), texto público. Código próprio.
2. **Tensão.** O SVF do FILTER é limpo e estável. O caráter do baixo
   subtrativo está noutro lugar: o grave que **afunda** quando a ressonância
   sobe, a saturação **dentro** do laço (o filtro que se comprime ao ser
   empurrado) e o diodo que range. Sem isso, o grave do Modular fica
   educado.
3. **Unidade.** O **corpo do timbre grave**: a região entre a fundamental e
   os primeiros harmônicos, e como ela reage à ressonância e ao volume.
4. **Percebe / conserva / transforma / recusa / esquece.** Percebe o áudio
   de entrada. Conserva o estado dos quatro integradores. Transforma o áudio
   em passa-baixas de 24 dB (LP4) e de 12 dB (LP2, a tomada do 2º degrau,
   com a mesma realimentação). Recusa ser um SVF a mais: não há passa-altas
   nem passa-banda (isso já é do FILTER). Esquece: nada além do próprio
   decaimento do filtro.
5. **Regra, recurso, risco, limite.**
   - Regra: a ressonância é realimentação da saída do 4º degrau.
   - Recurso: a saturação (tanh) no nó de soma da entrada, onde a escada
     real satura. Ela segura a auto-oscilação num ciclo-limite, em vez de
     deixá-la explodir.
   - Risco 1: o grave some quando a ressonância sobe (o ganho em DC cai para
     1/(1+k)). A resposta é **COMP**, compensação do grave, visível como
     knob: em 1, o ganho em DC volta a 1.
   - Risco 2: aliasing da saturação. A resposta é o laço a 2×
     (`Oversampler2x`).
   - Limite: auto-oscila em RESO = 1 e não passa disso (tanh).
6. **Consequência.**
   - O baixo subtrativo: OSC → LADDER → VCA, com um envelope no FC.
   - Com RESO alta, a ressonância vira um oscilador senoidal afinado em
     1 V/oct pelo FC.
   - DRIVE empurra o laço, e o som comprime e encorpa.
7. **Diferença no Modular.** O FILTER é um SVF de três tomadas com o
   caráter na relação entre elas (SPREAD). O WASP é um SVF que satura de
   outro jeito. O LPG é vactrol. Nenhum é escada.
8. **Diferença para o resto (Cardinal e família).**
   - O `MoogFilter` do Rasgo Synth Studio é um Stilson–Smith com ganho de
     compensação acertado de ouvido. Aqui a compensação é **exata** (em
     COMP = 1, o ganho em DC é exatamente 1).
   - Os módulos do Cardinal (kocmoc LADR/DIOD, Bidoo lIMbO) separam escada
     de transistor e escada de diodo. Aqui o **ACOPLAMENTO** entre os
     degraus é um knob contínuo, DIODO:
     - 0 = degraus isolados (transistor);
     - 1 = cada degrau carrega o vizinho (diodo);
     - no meio, escadas que não existem em circuito.
   - **A ressonância fica afinada no FC em qualquer DIODO.** O acoplamento
     desloca a frequência do pico, e o módulo corrige o corte para que o
     pico, e a auto-oscilação, caia na nota que o FC pede. RESO = 1 é o
     limiar de auto-oscilação calculado para cada acoplamento, não um
     número fixo. Assim, RESO = 0,9 significa a mesma proximidade do
     limiar em qualquer escada.
9. **Escuta.**
   - Êxito:
     - o baixo encorpa, sem sumir, quando RESO sobe com COMP ligado;
     - o DIODO vai do redondo ao ácido sem mudar de nota;
     - em RESO = 1, ouve-se uma senoide afinada.
   - Fracasso:
     - chiado de aliasing com DRIVE alto em notas agudas;
     - estalo ao girar o CUT;
     - o volume dispara com DRIVE.
   - Nominal: usar a escada como um FILTER qualquer, com a ressonância
     baixa e sem envelope. A receita didática precisa mostrar o baixo com
     envelope no FC.
10. **Proveniência.**
    - O circuito de R. Moog e a escada de diodos: conhecimento público.
    - A discretização ZDF/TPT: Zavalishin (texto público).
    - A compensação do grave como knob: conceito visto no Bidoo lIMbO e no
      kocmoc DIOD (GPL), nenhum código lido ou copiado.
    - O `MoogFilter` do Studio (mesmo autor): lido como referência, não
      portado.
    - O acoplamento contínuo e o pico afinado: desvio RASGO.

## Comportamento

- **Modelo linear.** Quatro integradores TPT com ganho g = tan(π·fc/fs):
  - ẏ₁ = ω(u − y₁) + c·ω(y₂ − y₁);
  - ẏᵢ = ω(yᵢ₋₁ − yᵢ) + c·ω(yᵢ₊₁ − yᵢ), para i = 2, 3;
  - ẏ₄ = ω(y₃ − y₄).

  Aqui c é o acoplamento (DIODO) e u = x − k·y₄. O sistema tridiagonal
  4 × 4 é resolvido exatamente a cada amostra (algoritmo de Thomas), sem
  atraso no laço.
- **Não linearidade.** u = tanh(u_lin), onde u_lin é a solução linear
  instantânea do laço (a forma "barata" de Zavalishin), com o laço inteiro
  a 2×.
- **RESO.** k = RESO · 1,03 · k_crit(c). O k_crit e a razão entre o pico e
  o corte são calculados pela resposta analógica, numa tabela de 33 pontos
  em c.
- **COMP.** A entrada é multiplicada por (1 + COMP·k): em COMP = 1, o ganho
  em DC é exatamente 1 com qualquer RESO.
- **DRIVE.** Ganho de entrada de 1 a 8 (≈ +18 dB) antes do tanh, com
  compensação parcial de nível na saída.
- **Entradas e saídas.**
  - Entradas: IN, FC (1 V/oct, exponencial), Q (soma à RESO).
  - Saídas: LP4 e LP2.
  - CUT é suavizado em 5 ms, como no FILTER.

## Na seed

Entra no rack da seed pela regra de 9/11 (todo módulo novo entra no rack).
Uma vaga dedicada na cadeia (por exemplo, como alternativa ao FILTER na voz
de baixo) fica para depois da escuta do autor.
