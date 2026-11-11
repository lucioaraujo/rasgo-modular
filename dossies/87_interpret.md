# 87 — INTERPRET: o intérprete melódico, com CHARACTER

Data: 11/11/2026. Estado: **em implementação** (fila aprovada pelo autor;
"avance em todos eles"). Origem: o `MelodicInterpreter` do Antitotem (código do autor, AGPL): conceito, não código — mesmo autor ou
coautoria creditada; conceito, não código.

## O que é

Lá o intérprete GERA a altura; aqui ele INTERPRETA as notas que chegam (o desvio obrigatório).

## Comportamento

- **As cinco perguntas do Método:**
  1. **O que percebe:** PTCH e GATE de qualquer sequenciador.
  2. **O que conserva:** as 8 notas recentes, o registro médio, a frase e os três eixos de caráter.
  3. **O que transforma:** a nota escrita em nota tocada: glide por intervalo, overshoot nos saltos, vibrato progressivo, articulação pelo contexto (marcato no salto, tenuto no ápice, staccato em duas notas rápidas, legato no resto), fôlego e brilho.
  4. **O que recusa:** inventar notas; ele só toca as que chegam.
  5. **O que esquece:** o fôlego volta numa pausa de 0,4 s; o caráter deriva devagar (DERIVA).
- **Saídas:** PTCH, AMP, BRT, FRASE e GT.
- **EXPR 0:** a nota crua.

## Teste (`tests/test_interpret.cpp`)

- o salto de oitava desliza 0,20 s, e o passo de 2 st 0,10 s;
- o overshoot passa do alvo ~5 cents e assenta;
- o vibrato aparece só na nota longa;
- os gestos saem do contexto;
- EXPR 0 é a nota crua.
- Duas correções antes do commit: o overshoot passou a entrar no alvo do glide, e o staccato passou a pedir duas notas rápidas.

## Na seed

Entra no rack da seed (regra de 9/11) na família DECISION. Um papel dedicado
fica para depois da escuta do autor.
