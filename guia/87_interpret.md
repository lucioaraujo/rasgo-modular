# INTERPRET — o intérprete melódico, com CHARACTER

**Módulo 87** · **Família:** DECISION
**Dossiê técnico:** [`../dossies/87_interpret.md`](../dossies/87_interpret.md)

---

## A ideia

Lá o intérprete GERA a altura; aqui ele INTERPRETA as notas que chegam (o desvio obrigatório).

- **As cinco perguntas do Método:**
  1. **O que percebe:** PTCH e GATE de qualquer sequenciador.
  2. **O que conserva:** as 8 notas recentes, o registro médio, a frase e os três eixos de caráter.
  3. **O que transforma:** a nota escrita em nota tocada: glide por intervalo, overshoot nos saltos, vibrato progressivo, articulação pelo contexto (marcato no salto, tenuto no ápice, staccato em duas notas rápidas, legato no resto), fôlego e brilho.
  4. **O que recusa:** inventar notas; ele só toca as que chegam.
  5. **O que esquece:** o fôlego volta numa pausa de 0,4 s; o caráter deriva devagar (DERIVA).
- **Saídas:** PTCH, AMP, BRT, FRASE e GT.
- **EXPR 0:** a nota crua.

Cada controle e cada jack estão explicados no LEARN do app, nos quatro
idiomas.
