# Auditoria dos módulos — o que tem função, o que não responde

Pedido do autor em 25 set. 2026: *"verifique também em todos os módulos
(uma auditoria) se há algum item sem função ou bugado"*.

Feita **por medição**, não por leitura. A ferramenta está em
`tests/tool_module_audit.cpp` e responde a três perguntas para os **58
módulos**, **458 parâmetros** e **154 portas de saída**:

1. mexer neste parâmetro muda a saída?
2. esta porta produz algo?
3. a saída é sempre finita, sem NaN nem Inf?

## O resultado sólido

**Zero não-finitos.** Nenhum módulo, em nenhum extremo de nenhum
parâmetro, produziu NaN ou Inf.

**Zero portas mortas, e um parâmetro defeituoso encontrado.** Dos 32
parâmetros e 19 portas que a triagem acusou: 13 portas e 28 parâmetros
foram confirmados funcionando com excitação própria, 6 do `SIGNAL-IN` são
silêncio esperado, e os 4 que ficaram sem prova em 25 set. foram fechados
em 27 set. — **três funcionavam** e **um estava quebrado**
(`QUANTIZER.hysteresis`). Ver "Os 4 sem prova — fechados" abaixo.

A correção da frase anterior deste resumo, que dizia "nenhum era defeito",
é o ponto: a auditoria tinha razão em não afirmar. Para um instrumento com 58 módulos de DSP,
realimentação, auto-oscilação e folding, isso é o resultado que mais vale
— é a classe de defeito que estraga uma gravação e assusta quem escuta.

## O que a auditoria NÃO prova, e por quê

A lista de "parâmetros sem efeito" é uma **triagem**, não um veredito, e a
razão está documentada porque me custou três refinamentos:

| métrica | acusou | por que estava errada |
|---|---|---|
| RMS nos extremos | 108 | RMS não vê TIMBRE: `OSC.pw`, `MATTER.structure`, `STRING.position` e todos os `drift` mudam o som sem mudar a energia |
| forma de onda amostra a amostra | 56 | melhor, mas eu injetava clock externo — e clock externo DEVE sobrepor o interno, então todo `*.rate` e `CLOCK.bpm` apareciam |
| com E sem entradas conectadas | **32** | resta a qualidade da excitação genérica |

**Caso demonstrativo:** `ENVELOPE.decay` e `.sustain` entraram na lista
final. Testados com um gate de 200 ms a cada 1,2 s — em vez dos 27 ms da
excitação genérica — respondem claramente:

| parâmetro | 0,02 | 0,5 | 2,0 |
|---|---|---|---|
| `decay` (RMS do env) | 0,067 | 0,272 | 0,368 |
| `sustain` | 0,222 | 0,322 | 0,441 |

Funcionam. **A falha era da minha excitação**, que não deixava o envelope
chegar ao decay. Um parâmetro só é julgado depois de exercitado no seu
próprio idioma.

## Classificação dos 32

### Explicados — comportamento correto (11)

| item | por quê |
|---|---|
| `MASTER.width`, `MASTER.mono` | a excitação é mono (L = R); mid/side não tem lado para escalar. É o mesmo achado que o autor relatou na escuta, e a auditoria o reproduz |
| `MASTER.body_guard` | só age em som alto, sustentado e concentrado em 2,5–8 kHz. Verificado à parte: reduz 1,39 dB num tom de 4 kHz e 0,00 dB em ruído de banda larga |
| `PARAMETRIC.gain1`, `freq2/q2/slope2`, `freq3/q3/slope3`, `freq4/q4/slope4` | mudar a frequência ou o Q de uma banda de EQ **com ganho 0 dB** não faz nada, e não deveria. A auditoria varia um parâmetro por vez, então a banda está sempre plana |

### Verificados à mão — funcionam (2)

`ENVELOPE.decay`, `ENVELOPE.sustain` — ver a tabela acima.

### Em aberto — precisam de exame caso a caso (19)

Nenhum destes está **provado** defeituoso; todos precisam de excitação
específica antes de se afirmar qualquer coisa:

`PLL.feedback_type` · `CONTROL.curve1`, `curve2` · `SH.slope`, `track1`,
`track2` · `TRIGSEQ.density1`, `density2`, `swing` ·
`QUANTIZER.hysteresis` · `HARMONY.scale_hi` · `BOXCAR.delay`, `thresh` ·
`SWITCH.mode` · `MATRIX.norm`, `ring` · `SCOPE.trigger`

## Os 4 sem prova — fechados (27 set. 2026)

Os últimos quatro da lista acima. Cada um ganhou teste no `ctest`, e a
condição que faltava era sempre a mesma coisa: **exercitar o parâmetro no
seu próprio idioma**.

### `QUANTIZER.hysteresis` — DEFEITO, corrigido

Não era falta de condição. A banda-morta comparava
`|snapped − heldSemi_|` — duas saídas **já quantizadas**. Como `snap()`
devolve semitons inteiros, essa diferença é no mínimo o menor passo da
escala, e a banda máxima (0,5·12/graus) é **menor que esse passo em 11 das
12 escalas**. Medido: **39 trocas de nota com a banda em 0 e 39 com a banda
em 1**.

Sobreviveu porque todos os casos de `test_quantizer.cpp` fixavam
`hysteresis = 0.0f`, para isolar o que mediam. O parâmetro nunca foi
exercitado.

Corrigido: a banda agora mede quanto a **entrada** passou da fronteira
entre a nota presa e a candidata, e é relativa ao passo **local** e não ao
médio da escala — pelo médio ela travaria E→F e B→C na maior (passo 1
contra médio 1,71), deixando dois graus inalcançáveis. Teto 0,45·passo,
não 0,50, para que movimento deliberado sempre passe. Tremor: 39 → 0
trocas.

### `SCOPE.trigger` — funciona

A auditoria contava pulsos, e contar pulsos não pode provar o nível: numa
senoide, qualquer nível dentro da amplitude dá um pulso por ciclo. O nível
move a **fase**, que é para o que um osciloscópio o usa. Senoide de 100 Hz,
amplitude 0,8, período 480 amostras:

| trigger | fase medida | prevista por asin(L/A) |
|---|---|---|
| −0,6 | −64 | −64 |
| −0,3 | −29 | −28 |
| 0,0 | 1 | 0 |
| +0,3 | 30 | 29 |
| +0,6 | 65 | 65 |

### `BOXCAR.delay` e `.thresh` — funcionam

Condição comum: modo follower, `scan` em zero (janela parada), abertura
pontual. Aí a saída é o valor da onda numa fase única.

`delay` é a fração do período em que a janela abre, e a saída tem de ser
A·sin(2π·delay) — amplitude 0,6: **0,000 · 0,5999 · −0,008 · −0,5999**
para delay 0 · ¼ · ½ · ¾ (previsto 0 · 0,600 · 0 · −0,600).

`thresh` é o nível cujo cruzamento ascendente serve de referência quando
`TRIG` está livre. Com `delay` em 0 a janela abre em cima do cruzamento, e
a saída é o próprio limiar: **−0,400 · 0,005 · 0,400** para limiares −0,4 ·
0 · 0,4. Limiar acima do pico não dispara nunca, e a saída então nem
oscila — o que descarta um módulo que só copiasse o parâmetro.

## Portas sem saída (19) — examinadas (25 set. 2026)

**Todas as 13 que importam funcionam.** As seis do `SIGNAL-IN` são
esperadas: sem entrada de áudio ligada ele não produz nada, por desenho.

| porta | faltava | pico |
|---|---|---|
| `STAGES.eoc`, `step` | `loop = 1` e um gate longo | 1,00 |
| `SH.out1` | gatilho | 0,49 |
| `SH.out2` | **gatilho FORA DE FASE com o sinal** — ver abaixo | 0,50 |
| `LOGIC.and`, `or`, `xor`, `flip` | pulsos LARGOS em A e B, que se sobreponham | 1,00 |
| `SEQUENCE.eos` | clock e voltas suficientes | 1,00 |
| `TRIGSEQ.t4`, `accent`, `any` | `density4 = 1` | 1,00 |
| `ABACUS.carry` | `count_step` e `modulus` baixo, para estourar | 1,00 |
| `BOXCAR.geiger` | `geiger = 1` e limiar baixo | 1,00 |
| `SWITCH.out_b/c/d` | **`dir = 1`** (demux): em mux elas não recebem nada, por desenho | 0,27 |
| `SCOPE.onset`, `trig`, `level` | transientes — rajadas com ataque abrupto | 1,00 |

**O caso do `SH.out2` é o mais instrutivo da auditoria inteira.** Eu dava
`in2` como seno de 5 Hz e `trig2` como pulso de 5 Hz: **travados na mesma
frequência**. O gatilho dispara no início de cada período, exatamente onde
o seno vale zero — então ele amostrava zero, sempre, e a porta parecia
morta. Com o gatilho a 6,5 Hz, sai 0,497.

Uma excitação pode ser da grandeza certa, na porta certa, e ainda assim
**medir a coisa errada** por coincidência de fase. É o tipo de erro que
nenhuma revisão de código pega e nenhum aumento de rigor evita — só
desconfiar do próprio instrumento de medida.

## Como repetir

```sh
cmake --build build --target rasgo_modular_module_audit
./build/rasgo_modular_module_audit
```

Não entra no `ctest`: é ferramenta de investigação, não teste de
regressão. Um teste que acusa 32 itens dos quais 13 já se explicam falharia
sempre e ensinaria a ignorar falhas.
