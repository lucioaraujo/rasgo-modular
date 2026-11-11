# ACCENT (ACENTO) — o campo de acento

**Família:** DECISION · **Módulo 75**
**Essência:** várias razões para acentuar uma nota, somadas numa ênfase só,
que move volume, brilho e articulação ao mesmo tempo.
**Dossiê técnico:** [`../dossies/75_accent.md`](../dossies/75_accent.md)
· **Fonte:** `src/dsp/Accent.hpp`

---

## A ideia

Um intérprete não acentua só com volume: a nota acentuada é mais forte,
mais brilhante e mais curta. E ele acentua por várias razões: o tempo forte,
o salto, o ponto alto da frase, a nota depois de uma nota longa, a síncope.
O ACCENT junta essas **causas** (cada uma com seu knob de peso) numa
**ênfase** de 0 a 1 e a entrega em várias saídas ao mesmo tempo.

As causas:
- **METR**: o 1º tempo de cada ciclo de CICLO notas é forte, e os outros
  pesam por hierarquia.
- **SALTO**: o tamanho do intervalo desde a nota anterior.
- **APICE**: a nota mais aguda das últimas 8.
- **EXT**: o acento que vem de fora, pela entrada ACC.
- **AGOG**: a nota que vem depois de uma nota longa.
- **SINC**: um acento de fora num tempo fraco.

E o que molda o conjunto:
- **HER**: o acento deixa uma cauda na nota seguinte.
- **SUPR**: depois de um acento forte, o seguinte custa mais, para que nem
  tudo vire acento.
- **ROT**: o tempo forte migra devagar; a métrica fica e a fase anda.

O visor mostra a ênfase no tempo. Cada controle e cada jack estão
explicados no LEARN do app, nos quatro idiomas.

## Como cabear

**O baixo ácido** (com o `LADDER`):
```
SEQUENCE → QUANTIZER (PTCH) → OSC (1V/O)
QUANTIZER (GATE) → ACCENT (GATE) e ENVELOPE (GATE)
QUANTIZER (PTCH) → ACCENT (PTCH)
OSC (SAW) → LADDER (IN) → VCA (IN1) → MIXER
ACCENT (SWP) → LADDER (FC)       os acentos seguidos sobem a varredura
ACCENT (DEC) → ENVELOPE (TIME)   o acento encurta a nota
ACCENT (VCA) → VCA (CV1)         e soa mais forte
```

## Receitas

1. **Fraseado numa linha.** Uma linha do `SEQUENCE` ou do `TURING`, com
   METR, SALTO e APICE no meio e a saída VCA no VCA da voz: a linha passa a
   respirar como uma frase.
2. **O acento que vem do relógio.** O ACC do `CLOCK` (euclidiano) no
   ACC, com SINC alto: os acentos que caem fora do tempo forte se destacam.
3. **Dois sons.** A saída GT no `DRUM`: um golpe só nas notas acentuadas.
4. **A métrica que anda.** CICLO 5, ROT 0,5: o tempo forte muda de lugar a
   cada poucos ciclos, e as notas continuam as mesmas.

## Se você conhece o Eurorack

Faz parte do papel do acento da TB-303 (filtro, VCA e decaimento ao mesmo
tempo, e o empilhamento) e dos módulos de "accent" de sequenciadores. A
diferença: aqui o acento é um **campo** com várias causas e várias saídas,
não um botão. A herança e a rotação vêm do Antitotem; um campo para vários
destinos, do Aquorbium.
