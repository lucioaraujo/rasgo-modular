# Dossiê — Módulo 36: Campo caótico de poço duplo (`CHAOS`)

**Família:** DECISION
**Estado:** **implementado — marco 3** (2026-09-05)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Chaos.hpp`, `tests/test_chaos.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.3`

## Estado da implementação (marco 3)

Fonte genuinamente CAÓTICA — diferente de tudo que já existia no
catálogo. `DECISION`/`TURING` SORTEIAM (pseudo-aleatório, mesmo que
estruturado); `DRIFT` soma ruído filtrado num passeio. `CHAOS` é uma
EDO não-linear de verdade: dois integradores (`x`, `y`) perseguem uma
força restauradora de poço duplo (`x − x³`, estável perto de `x≈−1` e
`x≈+1`).

- **`rate`** (0,02–400 Hz) — velocidade de evolução; cobre de CV bem
  lenta a textura de áudio, o mesmo eixo que ENERGIA escala noutros
  módulos do Antitotem;
- **`drive`** — força do puxão pros poços E tamanho do "chute"
  periódico (ver abaixo);
- **`damping`** — energia perdida por passo; baixo deixa o sistema
  balançar largo e imprevisível, alto assenta num poço só;
- **`freeze`** — segura a trajetória exatamente onde está;
- **chute periódico**: sem forçamento, a EDO é determinística — uma
  vez orbitando um poço, nunca alcança o outro (medido: um teste que
  varre DRIVE/DAMPING sem chute dá sempre saída de um sinal só). A
  cada ciclo de `rate`, um impulso aleatório empurra `y` — é isso que
  deixa a trajetória cruzar `x=0` de vez em quando, o "genuinamente
  imprevisível" do módulo;
- **`reseed`** (trigger) — pula pra um ponto aleatório novo, não espera
  o próximo chute.

Saída sempre `[−1,1]` (os estados internos `x`/`y` são clampados mais
largos, dando fôlego à dinâmica). Determinístico (RNG semeado xorshift64*).
Sem alocação/lock/IO em `process()`.

Desvio Rasgo: porta `reseed` como trigger de grafo (não método do host);
`rate_mod` como CV (convenção do Rasgo); painel e parâmetros próprios.

**Validação:** `tests/test_chaos.cpp` — **7 funções OK** (saída sempre
finita e em `[-1,1]`; a trajetória visita os dois poços, não fica presa
num só; `freeze` segura exatamente parado; `reseed` faz a trajetória
divergir de uma tomada de controle sem reseed — comparação de
trajetórias, não só o salto instantâneo, porque um sorteio pode calhar
perto do valor anterior por acaso; determinismo; grafo `CHAOS → FILTER`;
painel fecha). **43/43 CTest** Debug + Release. 5 renders de exemplo
byte-idênticos (nenhum usa `CHAOS`). Painel 8 HP, 0 sobreposições.
Catálogo: família DECISION, junto do `DECISION`/`DRIFT`/`ABACUS`.

**Pendências:** não adicionado ao `PatchSeed.hpp` (não aparece em seed
aleatório ainda — só via paleta); painel compila, não testado
visualmente.

**Não commitado.**
