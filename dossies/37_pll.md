# Dossiê — Módulo 37: Oscilador de malha de fase / VCO caçador (`PLL`)

**Família:** SOURCE
**Estado:** **implementado — marco 3** (2026-09-05)
**Padrão:** `AQUORBIUM/MODULE_DEVELOPMENT_STANDARD.md`
**Arquivos:** `src/dsp/Pll.hpp`, `tests/test_pll.cpp`
**Candidato:** `PESQUISA_MODULOS.md §2.3` (fecha os 2 últimos candidatos
de oscilador: "PLL/soft-sync" e "rede de feedback selecionável")

## Estado da implementação (marco 3)

Pedido explícito do autor: não um modo a mais dentro do `OSC`, um
**segundo oscilador sofisticado**, com itens que o primeiro ainda não
tem. Toca livre e sozinho sem nada plugado (`FREQ`/`FINE` próprios) —
é um VCO de verdade, não um efeito que depende de outro módulo. Com uma
referência de fase plugada (`REF` — espera uma serra bipolar, ex.:
`OSC.saw`), a taxa própria se CURVA pra perseguir a referência em vez
de resetar duro (diferente de `OSC.sync_enable`, que é hard sync).

- **`shape`** — morph contínuo seno↔triângulo↔serra↔quadrada (PolyBLEP),
  1 saída de forma variável em vez das 5 saídas simultâneas do `OSC`;
- **`ratio`** (0,03–8×, desvio Rasgo — o Antitotem só persegue 1:1) —
  trava em sub-harmônicos/harmônicos da referência; na ponta baixa
  (perto de 0,03×) alcança o estudo do "OSC4" do Antitotem (divisor bem
  fundo) sem precisar de outro módulo — quase um divisor de clock via
  malha de fase;
- **`lock_gain`** (0–1, exposto ao usuário — o Antitotem usa uma
  constante fixa 0,35) — força da perseguição;
- **rede de feedback selecionável** (`feedback_type`, 6 posições:
  direto/retificado/capacitivo/pulso/"transistor"/refluxo) — modula a
  FASE (não a frequência) só na leitura da forma, nunca entra no
  acumulador — era o candidato "rede de feedback selecionável",
  adiado antes por redesenhar o `fm_amount` do `OSC`; aqui é novo, sem
  esse conflito;
- **`ring`** — heterodino `saída × referência` (o mesmo que o estudo faz
  entre OSC5 e OSC A); fica em 0 sem `REF` (nada pra multiplicar);
- **`lock`** (saída de controle, desvio Rasgo) — 0..1, quão perto o
  detector de fase está de zero; um módulo pode SEGUIR o próprio estado
  de travamento (mesmo espírito do `gainReductionDb()` do `MASTER`).

**ALCANCE DE CAPTURA (medido, não hipotético):** a correção de fase é
limitada a ±0,9 (segurança). Isso limita matematicamente o quão longe
`FREQ` pode estar do alvo (`REF`×`RATIO`) e ainda travar EXATO: um
descompasso que pediria correção além de ±0,9 só se aproxima (caça de
verdade, nunca trava) — sonda: 150 Hz perseguindo um alvo de 300 Hz
(correção necessária = −1, fora do alcance) estabiliza perto de ~270 Hz,
nunca exatamente 300; 220 Hz pro mesmo alvo (correção ≈ −0,36, dentro
do alcance) trava exato. Não é bug — é o mesmo "capture range" limitado
de um PLL analógico de verdade, e é o que dá a instabilidade de "caça"
que o próprio conceito promete.

**Bug pego e corrigido durante a validação:** a primeira versão tinha o
sinal do erro de fase invertido (`esperada − própria` em vez de `própria
− esperada`, a convenção do estudo) — a correção puxava pro lado
ERRADO, afastando a taxa da referência em vez de aproximar. Achado
porque a sonda de travamento (`FREQ` longe do alvo, `LOCK_GAIN` alto)
falhava — a taxa medida deveria se aproximar do alvo e não se
aproximava.

Determinístico sem referência (nenhum RNG). Sem alocação/lock/IO em
`process()`.

**Validação:** `tests/test_pll.cpp` — **9 funções OK** (toca livre e na
própria FREQ sem `REF`; trava na referência dentro do alcance de
captura; `RATIO`=2 trava numa oitava acima, não 1:1; `LOCK` sobe quando
há referência e fica em 0 sem ela; as 6 redes de feedback ficam
limitadas e finitas; `RING` fica em 0 sem `REF`; determinismo; grafo
`OSC.saw → PLL.ref`; painel fecha). **44/44 CTest** Debug + Release
(43→44). 5 renders de exemplo byte-idênticos (nenhum usa `PLL`). Painel
16 HP, 0 sobreposições. Catálogo: família SOURCE, junto do `OSC`.

**Pendências:** não adicionado ao `PatchSeed.hpp` (só via paleta, não
em seed aleatório ainda). Painel compila, não testado visualmente.

**Não commitado.**
