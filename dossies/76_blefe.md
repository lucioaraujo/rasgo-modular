# 76 — BLEFE: o jogo do sinal

Data: 11/11/2026. Estado: **em implementação** (estudo da família §4.8.3,
aprovado: "excelente, gosto de todos"; "pode avançar nos módulos
restantes"). Origem: `BluffSignal` do Rasgo Synth Studio (mesmo autor;
conceito, não código).

## Portão conceitual

1. **Território.** A teoria dos jogos de informação incompleta. Nas
   palavras do autor, no Studio: "blefe exige três coisas: informação
   incompleta + expectativa + risco"; "um número aleatório não possui
   intenção. o blefe possui intenção percebida".
2. **Tensão.** No Modular, acaso é sorteio. O BLEFE dá ao acaso uma
   **história**: um sinal promete, a promessa se cumpre ou não, a confiança
   se constrói e pode ser traída.
3. **Unidade.** A **relação** entre duas vozes: quem sinaliza e quem
   responde.
4. **Percebe / conserva / transforma / recusa / esquece.**
   - Percebe gatilhos.
   - Conserva a confiança (média móvel) e se ela já foi construída.
   - Transforma cada gatilho num sinal que soa igual seja verdade ou blefe.
   - Recusa: depois da traição, a resposta se cala.
   - Esquece: pela memória da média móvel, e pelo perdão (RST).
5. **Regra, recurso, risco, limite.**
   - Regra: a verdade confirma depois da janela, o blefe não.
   - Recurso: a confiança.
   - Risco: a traição, que só existe depois de confiança construída (> 0,65)
     e cair abaixo do LIM.
   - Limite: uma traição por ciclo de confiança.
6. **Consequência.** O SIG dispara um som, sempre o mesmo, e o CONF dispara
   outro. A RESP abre a voz que responde na medida da confiança e se cala
   depois da traição: a forma da peça muda com a história do jogo.
7. **Diferença no Modular.** O DECISION sorteia, o MARKOV aprende, e nenhum
   tem **expectativa** nem **memória de confiança**. Primeiro módulo de
   jogo do Modular.
8. **Diferença para o Studio.** Lá a traição é para sempre (a peça acaba).
   Aqui o patch continua, e o RST é o **perdão**, a reorganização. As
   saídas são cabos: o músico escolhe quem sinaliza e quem responde. A
   verdade escondida sai num jack (VERD), para quem quiser trapacear.
9. **Escuta.**
   - Êxito: com VERD alto, a voz que responde cresce. Quando VERD cai, ela
     hesita, depois se cala de vez, e isso se ouve como uma decisão.
   - Fracasso: soar como um gate aleatório qualquer.
   - Nominal: só o SIG cabeado.
10. **Proveniência.** Conceito do Studio (mesmo autor). Média móvel e
    limiares: código próprio.

## Comportamento

- **Sinal:** cada gatilho em IN vira um SIG (10 ms). A verdade é sorteada
  com probabilidade VERD (xorshift semeado por SEED). Um sinal novo resolve
  o pendente.
- **Resolução:** na janela JAN, se era verdade, sai um CONF. A confiança
  segue `trust += MEM·(resultado − trust)`.
- **Saídas:**
  - CNF: a confiança, como CV;
  - RESP: um pulso por resolução com altura = confiança (0 depois da
    traição);
  - TRAI: o instante da traição.
- **RST:** confiança 0,5, sem traição.

**Teste** (`test_blefe`):
- todo gatilho vira um sinal;
- a taxa de verdade segue VERD (0,52 com 0,5);
- só a verdade confirma;
- o honesto não trai;
- o mentiroso desde o início não "trai" (não havia confiança);
- o honesto que passa a mentir trai uma vez, e a resposta se recusa;
- o RST perdoa;
- a mesma SEED dá o mesmo jogo.
