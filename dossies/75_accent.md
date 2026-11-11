# 75 — ACCENT (ACENTO): o campo de acento

Data: 11/11/2026. Estado: **proposta, aguarda o autor**. Só o dossiê, sem
código. Origem:
- o item aprovado na fila ("aprovo todas… sequencia normal");
- `ESTUDO_familia_rasgo_para_o_modular.md` §4.5, nuance 13 ("o acento é
  multidimensional");
- `PESQUISA_cardinal_baixo_e_osciladores.md` §2 e §8.2 (a gramática do
  baixo sem uma "voz de baixo pronta").

## O que já existe (lido antes de propor)

- **Antitotem**, `docs/PESQUISA_ACENTUACAO_GENERATIVA.md` (CRI-ACC-001):
  - As sete famílias de acento: dinâmico, métrico, agógico, tímbrico,
    articulatório, de altura, estrutural. É a taxonomia do acento fenomenal
    de Lerdahl & Jackendoff (1983).
  - Lá já foram feitos: a força contínua, a **herança** (`accentTail`: 0,3
    no tempo forte, cai pela metade a cada passo), a **rotação** (qual passo
    é forte migra devagar) e o acento em dois destinos (filtro e forma da
    onda).
  - Ficou por fazer: hierarquia de níveis, supressão, acento por intervalo
    e registro, combinação de causas (`accent_source`).
- **Aquorbium**, `geometricAccent`: um único valor contínuo alimenta vários
  destinos ao mesmo tempo (dureza, densidade, irregularidade, excitação). É
  o "Accent Field" sem o nome.
- **Rasgo Synth Studio**, `AcidBasslineVoice`: um flag, dois destinos
  (ganho 0,6 + 0,4·acc e profundidade do envelope do filtro).
- **Cardinal**, Autinn Bass (lido na pesquisa do grave): o acento da 303
  encurta o decaimento do filtro (~200 ms), sobe o VCA ~60 % e
  **empilha** acentos seguidos (o "stacking" da Devilfish).
- **No Modular**, já há **fontes** de acento:
  - o `accent` do CLOCK (euclidiano A/B);
  - o `accent` do TRIGSEQ;
  - o `accent` do PHRASE;
  - o `accent` do BOUNCE (a força do quique).

  O DRUM tem uma **entrada** de acento. Mas, módulo a módulo, o acento é um
  ganho. Nenhum combina causas nem distribui a mesma ênfase para vários
  destinos.

## Portão conceitual

1. **Território.**
   - Acento fenomenal, métrico e estrutural: Lerdahl & Jackendoff, *A
     Generative Theory of Tonal Music* (1983).
   - Estruturas de acento combinadas pelo intérprete (melódico, métrico):
     C. Drake & C. Palmer, *Music Perception* 10(3), 1993.
   - Acento agógico: tradição atribuída a H. Riemann.
   - A gramática de acento da TB-303: o acento mexe no filtro, no VCA e no
     decaimento ao mesmo tempo, e acentos seguidos empilham.
2. **Tensão.** No Modular o acento é um botão de volume. Um intérprete
   acentua com várias coisas ao mesmo tempo (mais forte, mais brilhante,
   mais curto) e por várias razões (o tempo forte, o salto, o ápice da
   frase). E um acento contamina o seguinte ou o impede.
3. **Unidade.** A **nota** dentro da frase: o peso que ela recebe, de onde
   ele vem e para onde vai.
4. **Percebe / conserva / transforma / recusa / esquece.**
   - Percebe a nota (GATE), a altura (PTCH) e um acento de fora (ACC).
   - Conserva a posição no ciclo métrico, a altura anterior, o ápice
     recente e a cauda do último acento.
   - Transforma tudo num valor contínuo de **ênfase**, entregue em várias
     saídas ao mesmo tempo.
   - Recusa gerar notas ou ritmo: não é um sequenciador.
   - Esquece: a cauda desbota a cada nota, e o ápice é só o da janela
     recente.
5. **Regra, recurso, risco, limite.**
   - Regra: ênfase = soma ponderada das causas, mais a herança, menos a
     supressão.
   - Recurso: as saídas paralelas.
   - Risco: tudo acentuado, ou seja, nada acentuado. A resposta é a
     **SUPRESSÃO** (refratário): depois de um acento forte, o seguinte
     custa mais.
   - Limite: ênfase de 0 a 1.
6. **Consequência.** Uma só causa move volume, brilho e articulação ao
   mesmo tempo, cada um no seu cabo. É o acento como gesto, não como
   número. Junto do LADDER, é a gramática do baixo ácido.
7. **Diferença no Modular.** As fontes existentes (CLOCK, TRIGSEQ, PHRASE,
   BOUNCE) dizem **quando**. O ACCENT diz **quanto**, **por quê** (as causas
   somadas) e **em quê** (as saídas).
8. **Diferença para a família.**
   - No Antitotem, o acento mora dentro do sequenciador, e no Aquorbium,
     dentro do motor.
   - Aqui ele é um módulo com causas e destinos em cabos visíveis, como
     manda a regra do Antitotem: "uma função só é ativa quando tem rota e
     controle".
   - Traz o que lá ficou por fazer: acento por **salto** e por **ápice**,
     **supressão**, combinação de causas.
9. **Escuta.**
   - Êxito: uma linha do SEQUENCE ganha fraseado. O ápice e o salto cantam,
     e o tempo forte pesa sem martelar. No baixo, os acentos seguidos
     "sobem" (o empilhamento).
   - Fracasso: tudo acentuado, ou o acento como um volume que pisca.
   - Nominal: só a saída VCA cabeada. A receita precisa mostrar dois ou
     mais destinos.
10. **Proveniência.**
    - Taxonomia: Lerdahl & Jackendoff; Drake & Palmer.
    - A herança e a rotação vêm do Antitotem; o campo único para vários
      destinos, do Aquorbium; o acento em dois destinos, do Studio. Todos
      do mesmo autor (conceito, não código).
    - O empilhamento da 303: comportamento documentado do circuito (visto
      no Autinn Bass, GPL; nenhum código copiado).
    - Código próprio.

## Comportamento proposto

**Entradas**
- **GATE**: cada borda de subida é uma nota.
- **PTCH** (1 V/oct): para as causas de salto e de ápice.
- **ACC**: acento de fora, contínuo ou gate (o CLOCK, o TRIGSEQ, o PHRASE
  e o BOUNCE servem).
- **RST**: zera o ciclo métrico e a cauda.

**As causas** (um knob de peso para cada uma, 0–1)
- **MÉTR**: posição num ciclo de **CICLO** notas (2–16). O primeiro tempo é
  o forte, e os outros recebem uma hierarquia 2-ádica: num ciclo de 8, o
  5º pesa ½, o 3º e o 7º pesam ¼.
- **SALTO**: o intervalo da nota anterior. Uma oitava ou mais conta como
  inteiro.
- **ÁPICE**: a nota é a mais aguda das últimas 8. É o ápice melódico, a
  família de altura que o Antitotem não fez.
- **EXT**: o valor de ACC.

**O que se soma**
- **HERANÇA** (0–1): parte da ênfase de uma nota passa à seguinte e
  desbota pela metade a cada nota, como no Antitotem.
- **SUPR**, supressão (0–1): depois de uma ênfase alta, a nota seguinte
  perde essa proporção. É o refratário.

**Saídas** (todas seguram o valor até a próxima nota)
- **ÊNF**: a ênfase, de 0 a 1.
- **VCA**: o ganho para um VCA, de 0,6 + 0,4·ênf até 1,0, com profundidade
  em PROF. É o Studio.
- **FC**: o brilho, em V/oct, de 0 a +2 oitavas × ênf. Vai no FC do LADDER
  ou do FILTER.
- **DEC**: a articulação, em V/oct negativo para o `time_mod` de um
  ENVELOPE: o acento encurta a nota, como na 303.
- **SWP**: o **empilhamento**. Um envelope curto por nota, com altura ∝
  ênf, somado a uma carga que vaza devagar: acentos seguidos sobem a
  varredura e um acento sozinho não. É a 303.
- **GT**: o gate passa só nas notas com ênf ≥ 0,5. Serve para disparar um
  segundo som só nos acentos.

**Painel**: 12 HP, sem visor. As causas na fileira de cima (MÉTR, SALTO,
ÁPICE, EXT); CICLO, HERANÇA, SUPR e PROF no meio; 4 entradas e 6 saídas
embaixo.

## Perguntas ao autor

1. **Causas.** Métrica, salto, ápice e externa. Ficaram de fora o
   **agógico** (a nota longa como acento, que pede saber a duração antes da
   nota acabar: só dá para acentuar a **seguinte**) e a **síncope**. Entram
   agora ou depois?
2. **Hierarquia em níveis** (0–3, nomeados): a proposta é contínua. Quer
   também uma saída em níveis (degraus) para patches que pedem
   "forte/médio/fraco"?
3. **Rotação** do Antitotem (qual tempo é o forte migra devagar): entra
   como knob ROT ou fica para o VARIA mexer no CICLO?
4. **Nome**: ACCENT no painel e ACENTO nos textos, como no TOM e no KEY?

## Na seed

Depois da escuta. Candidato natural: a voz de baixo com o LADDER, com o
ACCENT ligado ao FC, ao VCA e ao DEC.
