# Dossiê — Módulo 28: Chave sequencial (`SWITCH`)

**Família:** ROUTE / UTILITY
**Estado:** **implementado — marco 3** (2026-09-03)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Switch.hpp`, `tests/test_switch.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.2`

## Estado da implementação (marco 3)

O rack roteia por CABO, mas não tinha o **roteador controlado** — a chave
que cicla entre fontes por clock/CV. É órgão de forma do sinal (não gera
altura como o `SEQUENCE`), essencial pra "cada compasso troca a voz",
"o filtro alterna entre dois osciladores", "um S&H escolhe de qual das 4
entradas amostrar".

- **`dir`** (0/1, def 0, 2026-09-04): `0` = **mux N→1** (`a`/`b`/`c`/`d`
  → `out`, a selecionada passa); `1` = **demux 1→N** (a entrada `a` vai
  pra `out`/`out_b`/`out_c`/`out_d` conforme o passo; as não-selecionadas
  deslizam pra 0 com o mesmo `slew`);
- **endereço** avança no `clock` (borda ↑), zera no `reset`, ou é dado
  direto pela CV `addr` (0–1 → 0..steps−1) — se `addr` está conectada ela
  manda;
- **`steps`** (2–4) — quantas posições na varredura;
- **`mode`** (0 forward · 1 pingpong · 2 random · 3 só-`addr`) — como o
  endereço anda; `random` usa xorshift semeado (determinístico);
- **`glide`** (0–1) — no ponto de troca, faz um crossfade entre a fonte
  velha e a nova em vez de corte seco (sem clique; e é musical);
- **`step`** (saída CV) — a posição atual normalizada (0..1), patchável
  pra seguir a varredura noutro lugar.

Desvio Rasgo: `spread` na ordem `random` (viés — a média de N sorteios
evita repetir a mesma posição) fica como candidato; por ora `random` é
uniforme com anti-repetição de 1.

Sem alocação / lock / IO em `process()`. Determinístico.

**Testes (12 funções, Debug + Release):** `dir = 1` (demux) → só 1 das 4
saídas ativa por vez, e o `forward` faz o sinal passar pelas 4 ao longo
do ciclo; `forward` cicla 0→1→2→3→0 (nº de
posições visitadas = steps); `pingpong` bate nas pontas e volta; `random`
não repete a posição imediatamente e cobre todas em N passos; `addr`
conectada seleciona direto (ignora o clock); `reset` volta a 0;
`glide = 0` → a troca é 1 amostra (a saída == a nova entrada no bloco
seguinte); `glide` alto → crossfade audível (derivada por amostra
pequena na troca); a entrada não-selecionada não vaza (silêncio de b/c/d
quando `step = 0`); `step` reflete a posição; tudo finito; dois renders
byte-idênticos; grafo `CLOCK → SWITCH.clock` · dois `OSC` em `a`/`b` ·
`SWITCH.out → FILTER`.

**Pendências (candidatos):** `hold` (as saídas não-selecionadas do demux
seguram o último valor em vez de ir a 0); número de entradas maior (8,
tipo A-152); `spread` no `random`.

---

## 1. Problema musical e papel no fluxo

Patch generativo interessante troca de material: a mesma linha melódica
tocada ora por um oscilador, ora por uma corda; o filtro alimentado ora
pelo ruído, ora pelo acorde; um S&H que amostra de fontes diferentes a
cada compasso. Sem `SWITCH` isso exige repatch manual. O `SWITCH` é a
peça que faz a **variação de roteamento** virar parte do fluxo
autônomo.

Papel: roteamento, entre várias fontes e um destino. `OSC`/`MATTER` em
`a`/`b` · `CLOCK → clock` · `out → FILTER` (a voz alterna). `DRIFT.a →
addr` (a fonte deriva devagar). No `seedPatch` v2 entra como um destino
de `gate` (clock) e várias fontes de áudio (as entradas) — roteamento
emergente.

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Doepfer A-151 / A-152** (chave sequencial / endereçada) | contador de endereço avançado por clock, ou CV direta; bidirecional | prática |
| **4ms SISM / Erica Pico SEQ** | slew/glide no ponto de troca | conceito |
| **Multiplexador CD4051** (teoria) | uma de N linhas conectada à comum pelo endereço binário | teoria pública |
| **`mode` de leitura do `SEQUENCE` do Rasgo** | forward/pingpong/random como parâmetro | código do autor |

## 3. Modelo — matemática, estados, extremos

Por amostra:
```
steps = clamp(round(steps_param), 2, 4)
if reset borda↑: pos = 0 ; ppDir = +1
if addr conectada:
  pos = clamp(round(addr[frame]·(steps−1)), 0, steps−1)
elif clock borda↑ && mode != 3:
  forward:  pos = (pos+1) % steps
  pingpong: pos += ppDir ; if pos==0||pos==steps−1: ppDir = −ppDir
  random:   novo = rng()%steps ; if novo==pos: novo=(novo+1)%steps ; pos=novo
  (na troca: prevPos = valor anterior de pos ; xfade = 1.0)

sel  = input[pos]  (0 se não conectada)
prev = input[prevPos]
xfade -= 1/(glide²·0.05·sr + 1)          # decai a taxa de `glide`
mix  = xfade>0 ? lerp(sel, prev, xfade) : sel
out += (mix − out) · slewCoef            # slew leve anti-clique (1 ms) sempre
step = pos / max(1, steps−1)
```

**Estados:** `pos`, `prevPos`, `ppDir`, `xfade`, `out_`, `prevClock`,
`prevReset`, `rng_`. Sem alocação.

**Extremos.** `steps = 2` → alterna a/b. `addr` com DC no meio → fica
travado numa posição (correto). `glide` no teto + clock rápido → a chave
"nunca assenta" (crossfade permanente) — vira um morph contínuo entre as
entradas (uso não-idiomático mas musical). `clock` e `addr` conectados →
`addr` manda (o clock é ignorado). Nenhuma entrada conectada → `out = 0`.
Entradas de áudio a taxa alta → o corte no ponto de troca (glide 0) faz
um clique — por isso o slew de 1 ms é sempre aplicado. Reset → `pos = 0`,
`xfade = 0`, RNG re-semeado.

## 4. Três modos obrigatórios

- **Autônoma:** nada conectado → `out = 0` (é roteador, não gera). Com
  `a`/`b` de dois `OSC` do rack e o relógio interno... não tem relógio
  interno — precisa de `clock`. (Aceitável: um roteador sempre tem uma
  fonte de tempo antes.) Sem `clock` e sem `addr` → fica em `pos = 0`
  (passa `a`).
- **Performance:** `steps`/`mode` ao vivo mudam o padrão de varredura;
  `glide` de "corte seco" (rítmico) a "morph" (contínuo); `addr` na mão
  = seleção manual.
- **Híbrida:** `CLOCK → clock` (troca por compasso); `DRIFT.a → addr`
  (a fonte deriva devagar); `LOGIC.flip → clock` (alterna a cada 2
  batidas); `SWITCH.step → *_mod` (o timbre segue qual fonte está
  tocando).

## 5. Portas, parâmetros, limites

**Entradas:** `a`, `b`, `c`, `d` (Audio), `clock` (Control, trig),
`reset` (Control, trig), `addr` (Control).
**Saídas:** `out`, `out_b`, `out_c`, `out_d` (Audio — `out_b..d` só
carregam no demux), `step` (Control).
**Parâmetros:** `steps` (2–4, def 4), `mode` (0–3, def 0), `dir` (0/1,
def 0), `glide` (0–1, def 0), `slew` (0–1, def 0,1).
**Limites:** `out` segue a entrada (não clampado — CV/áudio passam). CPU:
detecção de borda + lerp + slew por amostra. Sem alocação.

## 6. Alternativas descartadas

- **Bidirecional (mux + demux) no marco 1:** o mux N→1 saiu primeiro; o
  demux entrou na 2ª camada (2026-09-04) como `dir` + 3 saídas de áudio
  (`out_b`/`out_c`/`out_d`). `dir = 0` = comportamento original.
- **Relógio interno:** um roteador sem fonte de tempo não roteia; herdar
  um `rate` interno seria controle fictício quando não há `clock`. O
  `pos = 0` sem clock é o comportamento honesto.
- **Corte sempre seco (sem slew):** um clique por troca a taxa de áudio
  é um defeito; o slew de 1 ms é transparente e resolve.
- **8 entradas já:** 4 cobre alternância e quaternário; 8 (A-152) é
  candidato quando o painel tiver largura.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** `forward` → `pos` visita 0..steps−1 em ordem e volta;
`pingpong` → 0,1,2,3,2,1,0,1…; `random` → nunca dois iguais seguidos,
todas as posições em ≤ 2·steps passos; `addr` conectada → `pos` segue
`round(addr·(steps−1))` e o clock não muda nada; `reset` → `pos = 0`;
`glide = 0` → no bloco após a troca, `out ≈` nova entrada; `glide` alto
→ |Δ out por amostra| pequeno na troca; entrada não-selecionada não
aparece em `out`; `step` = `pos/(steps−1)`; dois renders byte-idênticos.

**Escuta:** a chave alternando dois timbres a cada compasso soa como
"variação" ou "corte"? `glide` médio dá um morph musical entre as
fontes? `random` com 4 vozes cria uma forma imprevisível mas coerente?
`SWITCH.step` seguindo o timbre (via `*_mod`) reforça a troca de
maneira audível?

## 8. Integração e painel

Classe `Switch` (`type()` = `"SWITCH"`), 7 entradas, 5 saídas, 5
parâmetros. `panel()` próprio (12 HP): knobs STEPS/MODE/GLIDE/SLEW +
toggle DEMUX, jacks A/B/C/D · CLK/RST/ADR in, OA/OB/OC/OD/STEP out,
Display (a posição atual
como barra). Testado isolado (varredura, endereço, reset, glide,
vazamento, determinismo) antes do patch. Cadeias canônicas:
`CLOCK → SWITCH.clock` · dois `OSC` em `a`/`b` · `SWITCH.out → FILTER`;
`DRIFT.a → SWITCH.addr` (roteamento que deriva). Adicionado ao catálogo
do painel (`apps/panel/ModuleCatalog.hpp`, família — não há ROUTE no
catálogo; entra em SEQUENCE junto do `TURING`/`SEQUENCE`).
