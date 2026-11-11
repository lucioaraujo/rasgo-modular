# BLEFE — o jogo do sinal

**Família:** DECISION · **Módulo 76**
**Essência:** um sinal que soa sempre igual, verdade ou blefe; quem
responde aprende a confiar e, se for traído, se recusa.
**Dossiê técnico:** [`../dossies/76_blefe.md`](../dossies/76_blefe.md)
· **Fonte:** `src/dsp/Blefe.hpp`

---

## A ideia

Um número aleatório não tem intenção, mas um blefe tem. O BLEFE transforma
cada gatilho num **sinal**, que soa sempre igual, seja verdade ou mentira.
Uma janela depois (JAN), a verdade se **confirma** e o blefe não. A cada
resolução, a **confiança** anda: sobe com verdades e cai com blefes, numa
velocidade dada por MEM. Depois de construída (acima de 0,65), se a
confiança cair abaixo do LIM, é **traição**. A partir daí a resposta se
recusa, até um RST, que é o perdão.

VERD é a taxa de verdade, e SEED fixa o jogo: a mesma semente dá as mesmas
verdades e mentiras. O jack VERD (saída) entrega a verdade escondida, para
quem quiser trapacear.

Cada controle e cada jack estão explicados no LEARN do app, nos quatro
idiomas.

## Como cabear

```
CLOCK (CLK)        → BLEFE (IN)
BLEFE (SIG)        → ENVELOPE (GATE)      o sinal: sempre o mesmo som
BLEFE (CONF)       → DRUM (TRIG)          a confirmação: um segundo som
BLEFE (RESP)       → VCA (CV1) de outra voz  ela responde enquanto confia
```

## Receitas

1. **A voz que desconfia.** VERD 0,9 no começo; depois, gire para 0,2. A
   voz que responde cresce, hesita e se cala de vez.
2. **Perdão no fim da frase.** O EOS de um SEQUENCE no RST: a cada frase,
   a relação recomeça.
3. **Trapaça.** A saída VERD num segundo ENVELOPE: um som que "sabe" antes
   de todo mundo.
4. **Confiança como timbre.** CNF no FC de um LADDER: quanto mais confiança,
   mais aberto o som.
