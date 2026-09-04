# Dossiê — Módulo 3: Relação de Cabo (`Cable::Relation`)

**Família:** CONNECTION (não é um nó — é comportamento da conexão)
**Estado:** **implementado — marco 1** (2026-09-01)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/core/SignalGraph.hpp` (`enum class Relation`,
`Cable::setRelation`, `Cable::applyRelation`, `relationName`/`relationFromName`),
`tests/test_signal_graph.cpp` (`testCableRelation`)

## Estado da implementação (marco 1)

Feito: a conexão (`Cable`) pode **processar** — lê um segundo sinal
(`companion`, sempre do bloco anterior pra não depender da ordem
topológica e permitir auto-relação) e o combina com o que atravessa,
misturado seco/molhado por `amount ∈ [0,1]`. Três relações:

- **`RingMod`** — `x·(1-a) + (x·companion)·a`. Modulação em anel na
  própria conexão: soma e diferença de bandas laterais, timbre metálico
  quando o companion é áudio, tremolo quando é sub-áudio.
- **`Fold`** — wavefolder de `x` dirigido pelo companion:
  `f = x·(1 + a·3·|companion|)`, depois dobra em ±1 (até 4 reflexões).
  O companion controla quanto o sinal "transborda" — de limpo a
  harmonicamente denso, de forma dinâmica.
- **`Difference`** — `x - a·companion`. Retificador de diferença /
  cancelamento: o companion subtrai do sinal. Com o companion sendo uma
  cópia atrasada ou filtrada de `x`, vira realce de transiente / filtro
  em pente.

Serialização: quando a conexão tem relação, a linha `cable` do patch
ganha `relation=<nome> companion=<nó>:<porta> amount=<v>`;
`deserialize()` reconstrói. Round-trip textual estável.

**Testes (4/4 alvos, 3 configs):** com companion constante 0,4 e sinal
0,5 — `None` passa 0,5; `RingMod` amount 1 → 0,20 (= 0,5·0,4);
`RingMod` amount 0,5 → 0,35 (seco/molhado); `Difference` amount 1 → 0,10;
`Fold` amount 1 → 0,90 (0,5·2,2 = 1,1 → dobra → 0,9). Round-trip:
`relation=ring companion=1:0 amount=0.75` sobrevive a
serialize → deserialize → serialize idêntico; `hasRelation()` e
`relation()` preservados.

**Pendências (candidatos, não controles fictícios):** cross-fade suave
entre relações ao trocar em performance (hoje troca dura); `Fold` com
oversampling (alias não medido ainda); mais relações (`Min`/`Max`,
`AND` lógico pra gates, `Compare`/`Slew` — Warps tem ~9 modos); companion
com atraso configurável em amostras (habilita pente / flanger na
conexão).

---

## 1. Problema musical e papel no fluxo

Num modular tradicional, combinar dois sinais (ring-mod, cross-fade,
wavefold dirigido) exige um **módulo** no caminho — mais um objeto, mais
dois cabos, mais HP. Warps (Mutable) inverteu isso: um módulo cuja
função É a relação entre as suas duas entradas. O Rasgo Modular leva a
ideia um passo adiante — se a **conexão já é um objeto** (Atlas §9-11,
com ganho, ruptura/cicatriz, condução probabilística), então a relação
entre dois sinais pode morar **na própria conexão**, sem nó nenhum.

Papel: qualquer cabo do patch pode virar um ponto de interação entre
dois fluxos. Não muda a topologia (continua um cabo de A pra B), mas o
que chega em B é A *em relação a* um terceiro sinal. "A relação é o
processo" (Atlas §39).

## 2. Fontes primárias e conceitos apropriados (não copiar código)

| Fonte | Conceito apropriado | Licença |
|---|---|---|
| **Mutable Warps** (`pichenettes/eurorack`) | um processador cujo parâmetro central (`ALGORITHM`) percorre um contínuo de formas de combinar dois sinais: cross-fade → ring-mod → diode ring → XOR → comparador → vocoder; o "internal oscillator" quando falta a 2ª entrada | STM32F4 = MIT — estudo do comportamento, sem código |
| **Ring modulation clássica** (Bode/Moog; DAFx) | produto de dois sinais = soma e diferença de frequências; quando um é sub-áudio → tremolo/AM | teoria pública |
| **Wavefolding** (`biome.odt` / DAFx23; Buchla 259/West-Coast) | dobrar o sinal em torno de ±1 gera harmônicos ímpares que dependem da amplitude — aqui a amplitude de dobra é *modulada* pelo companion | técnica pública |
| **Mannequins / princípio "3 tomadas"** (já no Módulo 2) | a relação entre saídas como material, não a saída isolada | conceito |

## 3. Modelo — matemática, estados, extremos

Por amostra, canal a canal, dentro de `Cable::process()`, no caminho
intacto, **depois** do ganho da conexão e **antes** do envelope de
condução (`dropGain_`):

```
x = source[c][n] · gain
se relation != None:
    y = companion ? companion[c][n] : 0
    x = applyRelation(x, y)
destination[c][n] = x · dropGain
```

`applyRelation(x, y)` com `a = amount`:

```
RingMod:     x·(1-a) + (x·y)·a
Fold:        f = x·(1 + a·3·|y|)
             repete até 4×:  se f>1: f = 2-f ;  se f<-1: f = -2-f ;  senão pára
             → f
Difference:  x - a·y
None:        x
```

**Estado:** nenhum — a relação é sem memória (o companion já carrega a
história). Isso mantém `Cable` barato e o determinismo trivial.

**Companion do bloco anterior.** `SignalGraph::process()` resolve o
companion de `previousOutputs_[nó][porta]`, não de `currentOutputs_`.
Consequência: 1 bloco de latência no sinal companion (inaudível pra
modulação; ~2,7 ms de defasagem a 48 kHz / bloco 128 pra companion de
áudio). Ganho: a relação nunca cria um ciclo na ordem topológica, e um
cabo pode usar como companion a saída do **próprio nó de destino**
(auto-ring-mod, auto-fold).

**Extremos.** `amount = 0` → idêntico a `None` (seco puro). `companion`
inexistente / fora de faixa → `y = 0` → `RingMod` zera, `Fold` vira
identidade, `Difference` passa direto (degradação segura). `Fold` com
`|x|` grande e `|y|` grande: o laço de 4 reflexões satura a excursão;
acima disso o valor é clampado implícito pelo limitador dos módulos a
jusante. Sinal não-finito no companion: tratado como qualquer amostra
(os módulos fonte já saneiam a própria saída).

## 4. Três modos obrigatórios

- **Autônoma:** sem 2ª entrada explícita, um cabo com relação usa como
  companion a saída de outro nó qualquer do patch (ex.: um LFO lento) —
  já dá tremolo / fold pulsante sem gesto nenhum.
- **Performance:** `amount` é o macro — varrer de 0 a 1 leva de "cabo
  normal" a "cabo que transforma" de forma contínua e audível. Trocar a
  `Relation` é um gesto discreto (hoje sem cross-fade — pendência).
- **Híbrida:** o companion é um sinal do grafo; modular a saída desse
  nó modula indiretamente a relação. Um `FUNCTION` em áudio como
  companion de um cabo `RingMod` que carrega outro `FUNCTION` = dois
  osciladores se multiplicando **na conexão**, sem módulo de FM.

## 5. Portas, parâmetros, limites

**Não tem portas nem parâmetros próprios** — é configuração de `Cable`:
- `setRelation(Relation, companionNode, companionPort, amount)`
- `relation()`, `companion()`, `relationAmount()`, `hasRelation()`
- `amount` clampado a [0,1].

**Limites:** custo O(1) por amostra por canal (1 mult + 1 lerp no
RingMod; ≤4 comparações no Fold). Sem alocação, sem estado. O companion
é um ponteiro pré-resolvido em `process()` (mesma mecânica de
`inputPtrs_`).

## 6. Alternativas descartadas

- **Fazer um nó `WARPS`** com duas entradas de áudio: funciona, mas
  recria o problema que Warps resolveu por hardware (mais um objeto no
  caminho). A relação-na-conexão é mais Rasgo e não custa topologia.
- **Companion do bloco atual** (`currentOutputs_`): daria latência zero,
  mas forçaria o companion a vir antes na ordem topológica e proibiria
  auto-relação. O bloco anterior é a troca certa.
- **Todas as ~9 relações de Warps já no marco 1:** contra a regra de
  profundidade. Três relações de naturezas diferentes (multiplicativa /
  não-linear dependente de amplitude / subtrativa) cobrem o espaço
  conceitual; o resto é 2ª camada.
- **Relação com estado (slew, filtro no companion):** útil, mas quebra a
  simplicidade "sem memória" — vira um mini-módulo. Candidato a um
  `companion` com pré-processamento configurável.

## 7. Critérios técnicos e perguntas de escuta

**Técnicos:** `RingMod` com companion senoidal produz bandas laterais
soma/diferença nas frequências previstas (não testado com FFT ainda —
marco 1 verifica a álgebra por amostra); `amount = 0` byte-idêntico a
`None`; round-trip de serialização idêntico; sem alocação (coberto pelo
teste geral do grafo); companion ausente não gera NaN.

**Escuta:** varrer `amount` num cabo `RingMod` soa como uma transição
contínua "seco → metálico"? o `Fold` dirigido por um LFO respira ou só
distorce? usar a saída do próprio destino como companion (auto-fold)
soa como um instrumento ou como realimentação descontrolada?

## 8. Integração e serialização

`Relation` vive em `SignalGraph.hpp` junto de `Cable`. `serialize()`
acrescenta `relation= companion= amount=` na linha do cabo só quando há
relação (patches sem relação ficam idênticos aos de antes — compatível).
`deserialize()` lê as três chaves e chama `setRelation`. O teste
`testCableRelation` cobre as três relações + round-trip. Um render que
exercite a relação numa peça entra junto com o Módulo 6 (a primeira
peça longa).
