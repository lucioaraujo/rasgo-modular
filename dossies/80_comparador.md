# 80 — COMPARADOR: o limiar como acontecimento

Data: 11/11/2026. Estado: **em implementação** (estudo da família §4.5:
"COMPARADOR com mixer polarizador", aprovado; "pode avançar nos módulos
restantes").

## Portão conceitual

1. **Território.** Duas peças do acervo do Antitotem, lidas como referência
   creditada de princípio (`PESQUISA_CMOS_LUNETTA.md`; nenhum esquema,
   valor ou PCB copiado):
   - o **mixer polarizador** (phono, 2010): um controle cujo centro é
     zero; cada entrada pode somar, opor-se ou anular-se;
   - o **mixer-comparador** (R. Brewster, 2002): MIX, INV e COMP — "o
     comparador converte uma relação contínua em decisão/evento".
2. **Tensão.** O Modular não tinha um comparador de uso geral. A relação
   entre dois sinais (quem está acima, quando se cruzam) não virava
   **evento**.
3. **Unidade.** A **relação** entre dois sinais.
4. **Percebe / conserva / transforma / recusa / esquece.**
   - Percebe A e B.
   - Conserva só o estado da histerese.
   - Transforma a relação em mistura polarizada (MIX/INV) e em decisão
     (COMP/CRUZ).
   - Recusa tremular com ruído (HIST).
5. **Regra, recurso, risco, limite.** Regra: COMP sobe acima de LIM + HIST/2
   e desce abaixo de LIM − HIST/2.
6. **Consequência.** Dois LFOs que se cruzam viram um ritmo. Um envelope
   que passa do limiar dispara outra voz. A diferença entre duas vozes
   (A − B) vira um terceiro sinal.
7. **Diferença no Modular.** O LOGIC trabalha com gates, o CONTROL soma sem
   zero exposto, o DECISION sorteia. Nenhum decide por limiar com
   histerese.
8. **Proveniência.** Princípios de phono e de R. Brewster (via acervo do
   Antitotem). Código próprio.

## Comportamento

- **MIX** = A·PA + B·PB + OFS, com PA e PB de −1 a +1 (zero no centro;
  padrão A + 1, B − 1: a diferença). **INV** = −MIX.
- **COMP**: gate com histerese.
- **CRUZ**: pulso de 5 ms em cada borda.
- **Entrada LIM** soma ao knob.
- Os knobs deslizam em 3 ms.

**Teste** (`test_comparador`):
- somar, opor, anular, e o OFS;
- num seno de 2 Hz com ruído: 558 subidas por segundo sem histerese e 2
  com HIST 0,3;
- CRUZ dá 4 pulsos em 2 ciclos.
