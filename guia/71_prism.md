# PRISM — o som através de um prisma

**Família:** SPACE · **Módulo 71**
**Essência:** separa o som em cores, atrasa cada cor e abre um arco-íris no estéreo.
**Dossiê técnico:** [`../dossies/71_prism.md`](../dossies/71_prism.md)
· **Fonte:** `src/dsp/Prism.hpp`

---

## A ideia

Num prisma, cada cor anda numa velocidade dentro do vidro e sai noutro
ângulo. O PRISM faz isso com o som: seis cores (do grave ao agudo), cada uma
atrasada pelo seu "índice" (DISP e CURVE) e posta num lugar do estéreo (SPRD).
Com CURVE positiva os agudos chegam depois, como no vidro; negativa, os graves
(o que o vidro não faz). FDBK devolve cada cor ao prisma: ecos que se separam
em cores a cada volta.

Cada controle e cada jack estão explicados no LEARN do app (ligue LEARN e
aponte o mouse), nos quatro idiomas.

## Receitas

1. **O golpe que se desfaz.** Um `DRUM` → `PRISM`, CURVE +1, DISP 200 ms,
   SPRD 1: cada batida escorre do grave ao agudo, da esquerda para a direita.
2. **Arco-íris que respira.** Um LFO lento → `DISP`.
3. **Cascata.** FDBK 0,7, DISP 80 ms, MIX 0,6: ecos coloridos.
