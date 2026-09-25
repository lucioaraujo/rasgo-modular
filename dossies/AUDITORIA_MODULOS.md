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
parâmetro, produziu NaN ou Inf. Para um instrumento com 58 módulos de DSP,
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

## Portas sem saída (19)

Também triagem. As seis do `SIGNAL-IN` são **esperadas**: sem entrada de
áudio ligada ele não produz nada, por desenho. As outras —
`STAGES.eoc`, `SH.out1/out2`, `LOGIC.and/flip`, `SEQUENCE.eos`,
`TRIGSEQ.t4`, `ABACUS.carry`, `BOXCAR.geiger`, `SWITCH.out_b/c/d`,
`SCOPE.onset` — são em boa parte saídas de **evento** (fim de ciclo, fim
de sequência, carry, onset) ou de **roteamento** (`out_b/c/d` só recebem
quando a chave aponta para elas). Precisam da condição que as dispara, que
a excitação genérica não produz.

## Como repetir

```sh
cmake --build build --target rasgo_modular_module_audit
./build/rasgo_modular_module_audit
```

Não entra no `ctest`: é ferramenta de investigação, não teste de
regressão. Um teste que acusa 32 itens dos quais 13 já se explicam falharia
sempre e ensinaria a ignorar falhas.
