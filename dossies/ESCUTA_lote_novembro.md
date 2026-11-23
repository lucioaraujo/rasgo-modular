# Roteiro de escuta — o lote de 20 a 23/11/2026

Data: 23/11/2026. Para o autor: tudo o que entrou de 20 a 23/11 e espera a
escuta antes da v0.1.12. Pela regra de 9/11 (estudo §4.7), novos recursos
de som esperam esta escuta. Cada item diz **onde ouvir**, **o que procurar**
e **a decisão** (fica · ajustar · tirar). Anote ao lado, do jeito que for
mais rápido; eu passo para os dossiês.

As seeds abaixo foram procuradas no código atual (seeds 1–3000, as menores
em que o módulo chega ao som). Digite o número na caixa ao lado de SEED.

## 1. A forma da peça — seeds orquestrais (FORM + TEMPO + cópias de vozes)

- **Seeds 3, 12, 13, 15** (orquestrais, 4 vozes no MIXER).
- A peça leva de 3 a 7 minutos (LEN do FORM): deixe tocar até o fim ou
  grave com REC e ouça depois.
- Procurar: começo, crescimento até um clímax e fim (a dinâmica pelo FORM,
  até 9 dB entre vale e clímax); o andamento respirando com a tensão e o
  ritardando no fim (TEMPO); o baixo uma oitava abaixo e as vozes de cima
  (STRING/MATTER copiados) seguindo a harmonia da voz principal.
- Decisão: a forma soa como forma? O baixo e as vozes de cima somam ou
  embolam? Se embolam: menos cópias, ou mais baixas no MIXER.

## 2. Os papéis novos na seed

| Módulo | Seeds | O que procurar |
|---|---|---|
| GENDYN (voz estocástica) | 55, 74, 98 | a textura de Xenakis num canal próprio, sem dominar |
| RUNGLER (coração) | 5, 7, 11 | os degraus do registrador movendo o corte do filtro |
| ELASTIC sem DRUM | 23, 37 | a corda de massas como voz pinçada pelo relógio |
| ELASTIC com DRUM | 54 | o golpe do DRUM virando um corpo inarmônico |
| LENS (lente) | 10, 30, 52 | o barramento passando pela lente, sem lavar o som |
| TAPE (fita) | 27, 92 | o caráter de fita no barramento, WEAR baixo |
| PRESSURE (gás) | 3, 9, 13 | os choques do pistão como gatilhos da percussão |
| REFRACT (raio) | 2, 9, 15 | o ângulo movendo o corte |
| INTERFERENCE (franjas) | 2, 9, 22 | as franjas disparando a percussão (polirritmo) |
| PREDATOR (ecologia) | 1, 4, 10 | presa e predador no corte do filtro |
| SWARM (cardume) | 9, 12, 23 | o relógio da contravoz vindo do cardume |
| ECLIPSE | 3, 9, 21 | o evento raro abrindo o fill da percussão (ouvir uns minutos) |
| SEASON (estação) | 2, 12, 17 | o modo do QUANTIZER e o andamento mudando com a estação |
| VOLTA | 1, 11, 15 | a melodia que muda e volta (no lugar do TURING) |
| SUM (duas mãos) | 2, 9, 21 | dois movimentos somados no corte do filtro |

Decisão por linha: fica · ajustar (o quê) · tirar o papel. A régua técnica
já passou em todas (nível, picos, silêncio); a pergunta aqui é musical.

## 3. Recursos novos à mão (A/B: o knob em zero e depois aberto)

- **VCA4 · SAT**: em 0 é o VCA4 de antes (idêntico, medido); subir e ouvir
  a saturação OTA.
- **QUANTIZER · B e MRPH**: MRPH 0 = o de antes; a 0,5 aparecem alturas
  entre as duas escalas (microtons); 1 = a escala B.
- **QUANTIZER · banco**: clicar no visor, escolher (sugestões: maqam Rast,
  um gamelão, um gênero grego enarmônico, uma escala não oitavante) e ouvir
  a mesma melodia em cada uma. Conferir também a janela: busca, rolagem,
  clique.
- **CLOCK · RATE**: um FUNCTION lento no RATE e ouvir o andamento acelerar
  e frear.
- **FORM e TEMPO à mão**: a receita do guia do site (Experimente).

## 4. Com outros programas

- **Plugin no Ardour** (VST3 ou LV2): tocar, salvar a sessão, reabrir; o
  transporte da DAW virando MIDI clock no SIGNAL-IN (CLK → EXT do CLOCK);
  automatizar VARIA e Master.
- **POLY-IN**: um teclado MIDI tocando até 4 vozes.
- **REC → .mid**: um patch com NOTE-OUT no caminho, gravar, abrir o `.mid`
  no Ardour ou no MuseScore (o primeiro teste com um leitor que não é o
  meu).

## 5. Painéis e estalos

- **Painéis novos**: TEMPO (16 HP), FORM (LOOP embaixo), SIGNAL-IN (8 HP),
  QUANTIZER, CLOCK, VCA4, e os 17 módulos novos — legíveis? Algo apertado?
- **Estalos do VARIA**: com o VARIA ligado, abrir o SOBRE e ler os blocos
  perdidos (ou abrir o app com `RASGO_DIAG=1`). Isso diz se ainda há xrun.
- **Visores do TRIGSEQ e do KEY** (achado de 23/11): o TRIGSEQ mostra só
  ~8 ms e quase nunca pega um pulso; no app ao vivo ele pisca ou fica
  vazio? Se sim, eu proponho uma janela maior.

## 6. Depois da escuta

Com as decisões, eu ajusto o que for pedido, um item por vez, e preparo a
v0.1.12 (envio dos commits, CI, release) quando o autor decidir.
