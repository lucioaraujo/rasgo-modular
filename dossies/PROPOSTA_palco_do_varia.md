# Proposta — o PALCO do VARIA (gerência inteligente de pan)

Data: 10/11/2026. Estado: **proposta, nada implementado**. Pedido do autor:
"há uma gerência de pan inteligente no varia?" → "sim, prepare a proposta";
e a revisão de uma decisão dele: "podemos rever essa minha decisão de o
varia não mexer no pan do mixer".

## 1. O que existe hoje (conferido no código)

- **MIXER**: 4 canais **mono**, pan de potência constante (sin/cos; no
  centro, 0,707 em cada lado). O canal inteiro vai para o lado, **grave
  incluído**.
- **Seed**: todos os pans nascem em 0 ("o autor abre o palco à mão").
  A largura do MASTER é sorteada entre 0,85 e 1,5.
- **VARIA** (`MotionEngine.hpp`): **não toca MIXER nem MASTER** ("o músico
  mistura, a engine não"). Os `spread` de FILTER/SPACE/PRISM/SWIRL/PULSAR
  andam como qualquer parâmetro livre, sem noção de palco. O VARIA já
  recebe a **energia do som** (0–1) e acelera a mão com ela.

## 2. O que a família já sabe (lido no código, 10/11)

| Instrumento | O que faz | A nuance |
|---|---|---|
| **Asterismus** `PerformanceSkyPan.hpp` | a posição x da estrela na Constelação vira o lugar no estéreo | **grave no centro**: abaixo de ~150 Hz por um polo, agudo = sinal − grave, soma exata; **nunca além de 70 %** de um lado; ESPAÇO 0 = mono exato; lei de potência igual normalizada para o centro manter o ganho anterior; posição alisada em ~30 ms |
| **Navalha 2** `AssistedPerformer.cpp` (AUTO MIX) | por frase, com probabilidade 0,08 + 0,22·intensidade | as duas fontes têm **lados opostos** (casa = ±(0,06 + 0,12·energia)); o pan novo = 0,72·atual + 0,28·casa + deriva — **inércia**, não salto; teto de 0,28 a 0,48 conforme a energia (nunca no extremo); largura 0,85–1,25; só balanço, pan e largura |
| **Aquorbium** `Ecosystem::updateMix` + `AquariumSpatializer` | o lugar de cada organismo **emerge de quem o observa**: quatro observadores fixos no palco (−0,78, +0,78, −0,26, +0,26), cada um pesa pelo quanto vê e escuta | o pan é **consequência da percepção**, não atribuído; o canal mais cheio é normalizado a 0,92; **profundidade** = passa-baixa (o longe é mais escuro), separada do pan |
| **Rasgo Synth Studio** `SpatialExpressionField.hpp` | intenção espacial vira destino (pan, largura, profundidade, deriva) | **consciente do mono**: um fator `monoSafety` encolhe pan e largura; largura máxima 0,85; deriva com velocidade limitada |

## 3. Princípios (o que a proposta respeita)

1. **O pan é relação, não sorteio**: as camadas ocupam lugares
   **complementares**; duas camadas não pesam o mesmo lado.
2. **O grave fica no centro** (Asterismus): mover um canal não pode fazer o
   grave pender nem sumir em mono.
3. **Nunca no extremo** (Navalha 0,48, Asterismus 0,70): o palco abre, não
   rasga.
4. **Inércia**: o lugar muda devagar e carrega o anterior (Navalha 0,72);
   alisado para não ouvir degraus.
5. **Abertura pela energia**: denso e forte → palco mais aberto; rarefeito
   → mais junto (Navalha: teto cresce com a energia).
6. **Mono seguro** (Studio): a soma mono não pode perder nem ganhar
   volume por causa do pan.
7. **O músico manda**: mexeu no PAN à mão, aquele canal sai do VARIA até
   você devolver (mesma regra que o VARIA já usa para parâmetros
   modulados por cabo).

## 4. A proposta

### 4.1 PALCO — uma camada nova do VARIA, ligável

Um botão **PALCO** ao lado do VARIA (ou um modo dele), desligado por
padrão até a sua escuta. Ligado, o VARIA passa a mover **só os quatro PAN
do MIXER** — nunca os ganhos, nunca o MASTER.

- **Casas complementares**: cada canal ativo recebe uma casa no palco,
  espalhadas pelo número de canais que soam (1 → centro; 2 → ±; 3 →
  esquerda/centro/direita; 4 → duas internas e duas externas, como os
  observadores do Aquorbium).
- **Quem vai para onde**: a casa mais central para a **voz principal**
  (canal 1, a espinha da seed); as externas para as camadas.
- **A mão caótica do VARIA** desloca cada canal em torno da sua casa,
  devagar, com inércia; os quatro se movem **em relação** (se um vai à
  esquerda, outro tende a compensar), sem cruzar a casa vizinha.
- **Abertura** = f(energia do som): de ±0,15 em silêncio a ±0,55 no
  máximo (teto 0,70, do Asterismus).
- **Alisamento** de ~50 ms no próprio MIXER. Conferido: hoje o MIXER **não
  alisa** ganho nem pan (o valor vale no bloco inteiro), então o
  alisamento entra junto — é o que evita o clique quando o pan anda.

### 4.2 Grave no centro — no MIXER, para todos

Hoje o canal vai inteiro para o lado. Proposta: o MIXER separa o grave de
cada canal (um polo em ~150 Hz, agudo = sinal − grave, soma exata) e só o
agudo segue o pan; o grave fica no centro. Vale com ou sem PALCO, também
para o pan à mão. **Muda o som de patches com pan aberto** — por isso é
item separado, para você aprovar ou não.

### 4.3 Fora desta proposta (pode vir depois)

- **Profundidade** (Aquorbium): longe = mais escuro e mais baixo. Seria
  um segundo eixo do palco; mexe em timbre e ganho, então fica para outra
  conversa.
- **O pan como consequência da escuta** (Aquorbium): o lugar emergir de
  observadores, não de casas. É mais original e mais caro; vale se o PALCO
  simples convencer.
- Os `spread` dos módulos continuam como estão.

## 5. Decisões para o autor

1. **PALCO como botão próprio ou como parte do VARIA** (ligado sempre que o
   VARIA estiver ligado)?
2. **Grave no centro no MIXER (4.2)**: sim, não, ou só quando o PALCO
   estiver ligado?
3. **Abertura máxima**: 0,55 (proposta), 0,48 (Navalha) ou 0,70
   (Asterismus)?
4. **A seed** continua abrindo os pans no centro, ou o PALCO já define as
   casas ao abrir?

## 6. Como validar (antes de pedir sua escuta)

- teste: a soma mono do MIXER com PALCO ligado tem o mesmo RMS que com
  todos os pans no centro (±0,5 dB);
- teste: nenhum pan passa do teto; nenhuma casa é cruzada;
- teste: mexer num PAN à mão tira aquele canal do VARIA;
- teste: grave (< 150 Hz) com o mesmo nível nos dois lados para qualquer
  pan (se 4.2 for aprovado);
- escuta: A/B do mesmo patch com PALCO ligado e desligado, igualado em
  RMS.

## Proveniência

Conceitos lidos no código do autor: Asterismus (`PerformanceSkyPan.hpp`),
Navalha 2 (`AssistedPerformer.cpp`, AUTO MIX — Glerm Soares / Lúcio
Araújo), Aquorbium (`Ecosystem.cpp`, `AquariumSpatializer.h`), Rasgo Synth
Studio (`SpatialExpressionField.hpp`). Lei de potência constante e
crossover complementar de um polo são técnica pública. Nada copiado; a
implementação, se aprovada, cita cada origem no cabeçalho.
