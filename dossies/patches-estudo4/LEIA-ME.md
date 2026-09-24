# Patches prontos — Estudo 4 (Matéria e espaço)

Abra pelo **`Ctrl+O`** dentro do app e aponte para esta pasta.

## Por que existem

O Estudo 4 do protocolo pedia para **montar à mão** patches com módulos
específicos. Na sessão de 23 set. 2026 o autor respondeu: *"não consegui
fazer também, consegue criar o seed para testar o que precisa?"*

A falha era do protocolo, não de quem o executou. Pedir que se cabeie dez
módulos antes de ouvir a primeira nota, num instrumento onde encontrar e
ligar módulo a módulo **é o trabalho inteiro**, transforma um estudo de
escuta em exercício de montagem.

Cada patch abaixo foi **medido antes de ser entregue**, porque patch de
teste que não exercita o que devia é pior que nenhum. Os números são de
20 s de render (40 s no caso do espaço), a 44,1 kHz.

## Os quatro

### 1. `estudo4-1-materia.rmp` — ressoadores no limite

STRING, MATTER e RESONATOR excitados juntos, com decaimento longo e
amortecimento quase nulo.

| pico | LUFS | amostras no teto | finito |
|---|---|---|---|
| −5,9 dBFS | −10,4 | 0 | sim |

**Escutar:** algo explode, trava ou emudece de vez? Alguma ressonância
foge de controle quando várias se somam?

### 2. `estudo4-2-espaco.rmp` — realimentação alta

SPACE → HALL → LOOPER em série, com `feedback` em 0,92 / decaimento 0,95
/ 0,95. É o encadeamento que mais convida à realimentação descontrolada.

| pico | LUFS | amostras no teto | finito |
|---|---|---|---|
| −2,4 dBFS | −14,6 | 0 | sim |

**Escutar:** a cauda cresce sem parar? Some? Vira zumbido? A medição diz
que é **estável e não silencia** — o ouvido decide se é musical.

### 3. `estudo4-3-direto-no-out.rmp` — POR FORA do MASTER

Um ENVELOPE cabeado **direto no OUT**, sem passar pelo MASTER. É o teste
dirigido da guarda de segurança do sink.

| pico | LUFS | amostras no teto | finito |
|---|---|---|---|
| **−1,0 dBFS** | −1,1 | **58,9%** | sim |

−1,0 dBFS é exatamente o teto (`0,891251`), e **59% das amostras estão
nele** — o recorte é constante e inegavelmente audível. Isso é
proposital: é o aviso de que falta um MASTER no caminho. O que a medição
garante é o resto — nunca estoura sem limite, nunca produz NaN.

**CORREÇÃO (24 set. 2026).** A primeira versão deste patch punha um
ENVELOPE direto no sink e media 368 amostras no teto em 20 s — **0,04%**.
O autor escutou e relatou "sem clipes ou estalos", com razão: o patch mal
TOCAVA o teto e não exercitava a guarda que devia testar. Medindo o sink
isolado fica claro o que faltava:

| sinal | amostras no teto | distorção |
|---|---|---|
| 0,89 (no teto) | 0,0% | 0,0% |
| 1,2 | 46,7% | 18,0% |
| 2,0 | 70,6% | 47,4% |
| 5,0 | 88,6% | 77,9% |

O sinal precisa **passar** do teto, não encostar nele. Um MIXER com
`out_gain` alto faz isso agora.

**Escutar:** recorta de forma suja e audível, mas sem estalo de estouro?
Depois cabeie o mesmo caminho através de um MASTER e compare — é a
diferença entre o limitador do MASTER e a guarda bruta do sink.

### 4. `estudo4-4-signal-in.rmp` — microfone ou instrumento

SIGNAL-IN entrando em SPACE **e** excitando uma STRING — o instrumento
acoplado a outro, não apenas "com efeito".

**Sai mudo sem entrada ligada, e isso é o esperado**: o SIGNAL-IN só abre
a entrada de áudio quando existe. Ligue microfone ou instrumento e toque.

**Escutar:** a STRING responde ao que entra como se fosse excitada por
ele? Há realimentação acústica se o microfone ouvir as caixas?

## Como registrar

Para cada um, anote no `dossies/VALIDACAO_v0.1.0.md`: o que ouviu, o
**LUFS integrado** e o **true-peak** (cartão SOBRE), e "nada a relatar"
quando for o caso — um estudo sem achado escrito é indistinguível de um
estudo não feito.

Grave com `./run --rec-both` se quiser comparar os dois taps.
