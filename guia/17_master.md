# MASTER — barramento de saída

**Família:** OUT · **Módulo 17**
**Essência:** o estágio final — cola a imagem estéreo, protege a saída,
dá o nível de master, e mostra o pico. É o fim da linha: a saída dele já
vai pra sua placa de som.
**Dossiê técnico:** [`../dossies/17_master.md`](../dossies/17_master.md)
· **Fonte:** `src/dsp/Master.hpp`

---

## A ideia

O `MIXER` soma e panora, mas entrega um sinal cru — pode ter DC do
patch, largura estéreo exagerada, picos acima de ±1. Faltava o estágio
final: colar a imagem, proteger a saída, dar o nível de master, e
mostrar quanto está chegando. Sem o `MASTER`, o instrumento toca sinal
sujo e sem controle de nível.

**Você não cabeia a saída do `MASTER` em lugar nenhum** — ela já está
ligada à placa de som. O `STANDBY` do cabeçalho é o `MUTE` deste módulo.

## Por dentro

`GAIN` é o nível final; depois dele, uma cadeia de proteções em série,
cada uma resolvendo um problema específico:

- **bloqueio de DC** (passa-alta ~5 Hz) — remove qualquer deslocamento
  constante do sinal (a mesma explicação do `CABEAMENTO.md` §3: DC não
  se ouve como tom, mas rouba margem do limitador à toa).
- **guarda ultrassônica** — um filtro que contém conteúdo **acima** do
  que se ouve diretamente. Esse conteúdo inaudível ainda pode causar
  problemas reais mais adiante (na conversão digital-analógica, em
  processos não-lineares de equipamento externo) mesmo sem você
  "escutá-lo" — a guarda existe pra segurança técnica, não estética.
- **`BODY` (governador de corpo)** — diferente de um EQ estático (que
  corta uma faixa o tempo todo), este só **entra em ação** quando
  detecta agudo alto **e** sustentado **e** concentrado numa faixa
  específica (~2,5–8 kHz — onde aspereza/estridência incomodam mais o
  ouvido) — uma espécie de compressor dinâmico focado só nesse
  sintoma específico, que não faz nada quando esse agudo não está
  presente ou é só passageiro.
- **limitador *look-ahead* de pico verdadeiro** — um limitador comum só
  reage **depois** que um pico já aconteceu, o que pode ser tarde
  demais pra evitar o estouro. "*Look-ahead*" significa que o módulo
  atrasa o áudio por uns 3 ms **de propósito**, dando a si mesmo tempo
  de "ver o futuro próximo" e começar a reduzir o ganho **antes** do
  pico chegar de verdade — uma redução suave e antecipada em vez de um
  corte reativo e abrupto. "Pico verdadeiro" (*true peak*) é a mesma
  ideia do `CABEAMENTO.md` §3: pega também os picos escondidos **entre**
  amostras, não só os valores discretos.

**`WIDTH` (mid/side), o que realmente muda:** todo par estéreo pode ser
decomposto em duas partes — o **mid** (a soma L+R, o que é **comum**
aos dois canais — o "centro" da imagem) e o **side** (a diferença L−R,
o que é **diferente** entre eles — a "largura" da imagem). `WIDTH`
escala só a parte **side**: em 0, ela desaparece (L=R, mono puro); em
1, fica como veio; em 2, é dobrada — a imagem fica mais larga sem
mudar o conteúdo mid (o "centro" da mixagem permanece intacto,
igualmente presente nos dois canais).

**`MONO`, o que ele revela:** forçar L=R=(L+R)/2 é literalmente
descartar o side e ficar só com o mid. Se alguma coisa **some** ou fica
muito mais fraca ao fazer isso, é porque parte do conteúdo estava em
**oposição de fase** entre L e R (uma parte cancelando a outra na
soma) — um problema real de compatibilidade que só aparece quando você
verifica.

## Os jacks, um a um

### Entrada

- **`IN`** (áudio) — a entrada estéreo. **Plugue aqui:** `MIXER.L+R`
  (o normal); ou, pra uma voz só, direto uma saída de áudio.

### Saídas

- **`OUT`** (áudio) — a saída final. **Já vai pra placa de som** (no
  seed, `MASTER.out → sink`). Você raramente cabeia isto.
- **`VU`** (level) (controle) — o pico de saída com decaimento (~300 ms),
  0..1. **Plugue em:** qualquer `_mod` — um módulo que **siga o próprio
  volume do master** (ducking global, um LED-como-CV).

## Os controles, um a um

**GAIN** (slider, −60 a +12 dB) — o volume final. Nos seeds nasce em
−6 dB. O medidor VU mostra o pico relativo ao teto de −1 dBFS; o
quadrado à direita acende quando o limitador teve que segurar algo.

**WIDTH** (0–2) — largura estéreo (mid/side). 0 = mono; 1 = normal;
2 = dobro de lado.

**BODY** (body_guard, 0–1) — o governador de corpo: só age em agudo
alto + sustentado + concentrado (protege de aspereza). 0 = off.

**MUTE** (chave) — silêncio com rampa de ~8 ms. É o `STANDBY` do
cabeçalho.

**MONO** (chave) — força L = R = (L+R)/2 (checagem de compatibilidade).

**DC** (dc_block, chave) — bloqueio de DC (passa-alta ~5 Hz).

**LIMIT** (chave) — o limitador *look-ahead* por pico verdadeiro.
**Deixe ligado** — é o que segura o teto.

## Como cabear

```
MIXER (L+R) → MASTER (IN)      (já vem feito no seed)
```
Sobe o `GAIN` até o VU chegar perto do topo sem o quadrado do limitador
acender o tempo todo.

**Ducking global:**
```
MASTER (VU) → CONTROL (invertido) → VCA (cv) de um pad
```
O pad abaixa quando a mistura fica cheia.

## Potencializar

- **Checar mono:** ligue o `MONO` de vez em quando — se algo some, há
  cancelamento de fase no patch.
- **Largura por seção:** um `FUNCTION` bem lento no `WIDTH` via
  `connectToParameter` — a peça "abre" e "fecha" o palco.
- **O limitador como efeito:** empurre o `GAIN` de propósito num trecho
  — o limitador "cola" a dinâmica (loudness).

## Se você conhece o Eurorack

Faz o papel de um módulo de saída com limitador (Intellijel Stereo Line
Out + um limitador, WMD/SSF, Befaco Out). A matriz mid/side é Blumlein;
o limitador *look-ahead* + true-peak vem do `OutputStage` do
NAVALHA/ANTITOTEM (código do autor). É o par do `MIXER` (#16).
