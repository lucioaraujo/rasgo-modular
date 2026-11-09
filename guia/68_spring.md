# SPRING — reverb de mola

**Família:** SPACE · **Módulo 68**
**Essência:** o tanque de mola do amplificador: o eco metálico, o "boing" e o estalo de quem chuta a caixa.
**Dossiê técnico:** [`../dossies/68_spring.md`](../dossies/68_spring.md)
· **Fonte:** `src/dsp/Spring.hpp`

---

## A ideia

Um reverb de mola é uma mola de verdade dentro do amplificador: o som entra
por uma ponta, viaja, volta, e cada ida e volta chega com os graves atrasados em
relação aos agudos — o "boing". O SPRING imita isso com um laço de atraso e
filtros que espalham as frequências no tempo. TENS é o comprimento da mola;
DRIP, quanto "boing"; DAMP, o brilho; DECAY, quanto dura. KICK sacode o
tanque.

Cada controle e cada jack estão explicados no LEARN do app (ligue LEARN e
aponte o mouse), nos quatro idiomas.

## Receitas

1. **Guitarra de surf.** Uma voz (ou `SIGNAL-IN`) → `SPRING`; MIX 0,4,
   DRIP alto, DAMP baixo.
2. **O chute.** Um `CLOCK` lento ou o `GT` do `BOUNCE` → `KICK`: o tanque
   estala sozinho.
3. **A mola que geme.** Um LFO lento → `TENS`: o eco desafina.
4. **Só a cauda.** `WET` → um canal do `MIXER` com `FILTER` antes: o tanque
   como instrumento.
