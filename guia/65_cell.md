# CELL — autômato celular

**Família:** TIME · **Módulo 65**
**Essência:** uma linha de células que nasce de novo a cada pulso, e o desenho dela vira ritmo e melodia.
**Dossiê técnico:** [`../dossies/65_cell.md`](../dossies/65_cell.md)
· **Fonte:** `src/dsp/Cell.hpp`

---

## A ideia

Um autômato celular é uma linha de casas acesas ou apagadas. A cada pulso,
cada casa olha para si e para as duas vizinhas, e uma **regra** (um número de 0
a 255) decide se ela fica acesa. Regras simples dão desenhos que nunca se
repetem (30, 110), triângulos (90), coisas que andam (184). O CELL lê esse
desenho com uma **cabeça**: a casa sob ela dá o gate; as três seguintes escolhem
a nota.

Cada controle e cada jack estão explicados no LEARN do app (ligue LEARN e
aponte o mouse), nos quatro idiomas.

## Receitas

1. **Ritmo que se reescreve.** `CELL.GT` → `DRUM`; RULE 30, RATE 6: um padrão
   que nunca se repete igual.
2. **Melodia celular.** `CELL.PTCH` → `OSC`, `GT` → `ENVELOPE` → `VCA`;
   RANGE 1. Gire RULE devagar.
3. **A linha que abre o filtro.** `DENS` → `CUT` de um `FILTER`: quanto mais
   vida, mais brilho.
4. **Renascer marca a forma.** `EOS` → `ADVANCE` do `HARMONY`: o acorde muda
   quando a linha renasce.
