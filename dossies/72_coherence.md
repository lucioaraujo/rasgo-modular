# 72 — COHERENCE (COERÊNCIA): a ordem que se rompe

Data: 11/11/2026. Estado: **em implementação** (decisão do autor: "siga com a
COERÊNCIA, escuto junto"). Origem: estudo da família §4.8 (Rasgo Synth
Studio, `CoherenceCollapse`) e §4.12 (nenhum equivalente no Cardinal).

## Portão conceitual (Método RASGO, as dez perguntas)

1. **Território e genealogia.** Teoria da informação (C. Shannon, 1948:
   entropia como medida de imprevisibilidade); expectativa musical (D. Huron,
   *Sweet Anticipation*, 2006: previsão, surpresa, habituação; L. Meyer,
   *Emotion and Meaning in Music*, 1956). No RASGO, o `CoherenceCollapse` do
   Rasgo Synth Studio (conceito, não código): "ordem extrema também rompe".
2. **Pergunta/tensão.** A suposição comum é que ruptura nasce do caos. Aqui
   ela nasce do contrário: quando uma linha fica previsível demais por tempo
   demais, a própria previsibilidade acumula pressão até romper.
3. **Unidade musical.** A **linha de alturas** (as classes de altura das
   últimas notas), não o timbre nem o volume.
4. **O que percebe, conserva, transforma, recusa, esquece.** Percebe cada
   nota (altura 1 V/oct + gate). Conserva as últimas N classes de altura (a
   janela). Transforma isso em **ordem** (1 − entropia normalizada) e em
   **pressão**. Recusa: nada sobre timbre/volume. Esquece: na ruptura,
   descarta a metade mais antiga da janela — a cicatriz.
5. **Regra, recurso, risco, limite.** Regra: ordem acima do LIMIAR por
   PACIÊNCIA notas seguidas → ruptura. Recurso: a pressão. Risco: romper
   demais (por isso a CICATRIZ, um período refratário). Limite: nunca mais de
   uma ruptura por nota.
6. **Consequência temporal e sonora.** Sozinho não soa: entrega um gate
   COLAPSO e CVs (ORDEM, PRESSÃO, SURPRESA) que o patch usa — resetar um
   sequenciador, chutar um pêndulo, disparar o GESTO do PALCO, abrir um
   filtro. A consequência sonora é do patch; o módulo decide **quando**.
7. **Diferença para o que existe no Modular.** O MARKOV tem SURP (a raridade
   de uma nota); o COHERENCE mede a **ordem da linha inteira** e acumula
   pressão ao longo do tempo. Nenhum módulo mede previsibilidade.
8. **Diferença para o mesmo território noutro instrumento.** No Studio, o
   colapso é um evento interno do motor que reorganiza o sequenciador. Aqui
   é um **módulo com cabo visível**: o músico escolhe o que a ruptura move
   (regra do Antitotem: "uma função só é ativa quando tem rota e controle").
9. **Como a escuta reconhece êxito, fracasso, efeito nominal.** Êxito:
   numa linha que se repete, ouve-se a tensão crescer (PRESSÃO num filtro,
   por exemplo) e romper num momento que soa como consequência, não acaso.
   Fracasso: rupturas que parecem aleatórias, ou que nunca vêm. Efeito
   nominal: um gate que ninguém liga — a seed precisa poder ligá-lo.
10. **Fontes e proveniência.** Shannon (1948), Huron (2006), Meyer (1956)
    como conceito; a cadeia ação → tensão → limite → ruptura → cicatriz com
    histerese e destino declarado da energia vem do Batruc (README,
    "Força, deformação, transferência e ruptura"). Código próprio.

## Comportamento

- **Entradas:** PTCH (1 V/oct), GATE (cada borda de subida = uma nota; sem
  cabo, uma nota nova é contada quando a classe de altura muda), RST.
- **Janela:** as últimas N notas (JANELA, 4–32). **Ordem** = 1 − H/log₂(k),
  com H a entropia de Shannon das classes de altura na janela e k = o
  máximo de classes possível (min(N, 12)). Uma nota repetida → ordem 1; as
  12 por igual → ordem 0.
- **Pressão:** a cada nota com ordem ≥ LIMIAR, sobe 1/PACIÊNCIA; com ordem
  abaixo, cai pela metade (histerese: a pressão exige ordem **seguida**).
- **Ruptura:** pressão ≥ 1 → COLAPSO (pulso de 10 ms), a pressão volta a 0
  (**destino da energia**: ela vira o pulso — nada fica guardado), a
  metade mais antiga da janela é esquecida, e por CICATRIZ notas a pressão
  não sobe (refratário).
- **Saídas:** ORDEM (0..1), PRESSÃO (0..1), COLAPSO (pulso), SURPRESA (por
  nota: −log₂ p da classe tocada na janela anterior, normalizada 0..1).
- Determinístico, sem sorteio, sem alocação no áudio.

## Painel (10 HP, a ver com o autor)

Visor com o histograma das 12 classes na janela, a barra de ORDEM e a de
PRESSÃO; knobs JANELA, LIMIAR, PACIÊNCIA, CICATRIZ; jacks PTCH, GATE, RST /
ORD, PRES, COL, SURP.

## Na seed

Entra no rack da seed (regra de 9/11: todo módulo faz parte). O passeio
pode ligá-lo como qualquer fonte de gate/CV; um papel dedicado (COLAPSO →
reset do sequenciador da voz, ou → FRASE do PALCO) fica para depois da
escuta.
