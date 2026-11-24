# Estudo — o Rasgo Modular no VCV Rack e no Cardinal

Data: 21/11/2026. Pedido do autor: "também verificar como o rasgo
funcionaria no VCV e cardinal". Estado: **diagnóstico e proposta**; nada
portado. A licença já está resolvida: `src/dsp/` e `src/core/` são
oferecidos também sob GPL-3.0-or-later (decisão de 20/11, `LICENSING.md`).

Fontes conferidas: o código do Cardinal 26.02 (a extração local; o Rack
embutido é o **2.4.1**), o `plugins/Makefile` e o `plugins/plugins.cpp` dele,
o código dos módulos do Modular, e o que já estava em
`DESENVOLVIMENTO.md` ("Módulos para VCV Rack / Cardinal") e no
`ESTUDO_cardinal_catalogo_e_licencas.md`.

## 1. Veredito

**É viável, e o Modular está bem preparado para isso.** Cada módulo é
autocontido: recebe blocos de entrada, devolve blocos de saída, e nada mais.
O que no Modular é do grafo — a modulação de parâmetro por cabo, o
barramento de qualidades (energia, brilho, densidade…), a relação e a
condução dos cabos — mora em `SignalGraph`, fora dos módulos. Nenhum módulo
lê arquivo nem depende do JUCE. Então o port é uma **casca** por módulo, sem
reescrever o DSP.

O que **não vai junto** é a ideia do instrumento: seeds, VARIA, MUDA/CRUZA,
o cabo como processo (relação, condução, ruptura), o LEARN como está e a
interface. No VCV/Cardinal, os módulos do Rasgo seriam **módulos entre
outros**, num rack que o músico monta à mão. O instrumento continua sendo o
aplicativo.

## 2. Como cada coisa se traduz

| No Rasgo | No Rack 2 | Como |
|---|---|---|
| `Signal::process` em blocos (até 256 amostras) | `Module::process` amostra a amostra | a casca junta entradas num bloco curto (16 ou 32 amostras), roda o módulo e entrega as saídas; custa **um bloco de atraso por módulo** (0,3–0,7 ms a 48 kHz). Bloco 1 funciona, mas é caro (os módulos leem parâmetros por nome a cada chamada) |
| áudio ±1 | áudio ±5 V | ×5 na saída, ÷5 na entrada |
| gate/trigger 0–1, limiar 0,5 | gate 0/10 V, Schmitt ~1 V | ×10 na saída; na entrada, ÷10 (o limiar 0,5 vira 5 V; aceitar a partir de ~1 V como o Rack pede ajuste por porta) |
| 1 V/oct, nota 60 = 0 V | 1 V/oct, 0 V = C4 | **igual** — sem conversão |
| CV de controle (faixas próprias) | ±5 V ou 0–10 V | escala por porta, declarada na casca |
| `ParameterDescriptor` (mín, máx, padrão, unidade) | `configParam` | direto, gerado do descritor |
| `Signal::panel()` (HP, posições em mm, rótulos) | painel SVG 3U (128,5 mm) + widgets | gerado das mesmas posições em mm; o Rack já usa 1 HP = 5,08 mm e 3U |
| visores próprios (14, em JUCE) | widgets NanoVG | reescrever; começar por um visor genérico de traço |
| LEARN (4 línguas) | descrição do parâmetro e da porta, em inglês | o texto em inglês do LEARN vira o tooltip de cada knob e jack |
| módulos mono | cabos polifônicos de até 16 vozes | começar mono; depois, polifonia por cópia (uma instância do DSP por canal) |
| `double` nas físicas e no MIDI clock | o Cardinal compila com `-fsingle-precision-constant` | desligar a opção para o plugin, como o Befaco e o SurgeXT já fazem |
| C++17 | o Cardinal usa C++11 por padrão | `-std=gnu++17` por plugin, também com precedente (Befaco, Eightfold, SurgeXT) |

## 3. Quais módulos

- **Portam como estão (a grande maioria, ~95):** fontes, filtros,
  transformações, moduladores, tempo, decisão, rota, espaço — inclusive os
  que só existem no Rasgo (óptica, física, ecologia, Xenakis, RUNGLER,
  INTERPRET, RELATE, VIA, KEY, COHERENCE, BLEFE…), que é onde o Rasgo traz
  algo novo ao catálogo do Cardinal.
- **Ficam de fora (o Rack já tem, ou são o encaixe com o aplicativo):**
  SIGNAL-IN, POLY-IN e NOTE-OUT (o Rack tem MIDI-CV e CV-MIDI), MIXER (há
  muitos), o nó de saída do patch. **MASTER** pode ir como limitador de
  saída, se o autor quiser.
- **Pedem adaptação:** SAMPLER e TURNTABLE (carregar arquivo pelo diálogo do
  Rack; gravar ao vivo funciona igual); CLOCK (seguir o relógio do Rack pela
  entrada EXT, como já faz).

## 4. Os dois caminhos de publicação

- **VCV Rack Library:** um repositório próprio (`rasgo-modular-vcv`, por
  exemplo) com `plugin.json`, `Makefile` do SDK do Rack e as cascas; a VCV
  compila para Windows, macOS e Linux e publica na biblioteca. Aceita GPL.
  Sem custo.
- **Cardinal:** o plugin entra como uma pasta em `plugins/` do Cardinal (um
  submódulo git), listado no `plugins/Makefile` e no `plugins/plugins.cpp`,
  com os nomes dos modelos prefixados (o Cardinal compila todos os plugins
  num só programa). Exige GPL-3.0-or-later (resolvido), dependências
  mínimas e nada de rede (o Modular já cumpre). O caminho é propor ao
  mantenedor (falkTX) depois de o plugin existir na biblioteca da VCV ou como
  projeto aberto estável.

O mesmo código de casca serve aos dois.

## 5. O que é decisão do autor

1. **Aparência dos painéis** no Rack — estética, passa pelo autor com imagem.
   Proposta técnica: o painel gerado das posições do `panel()`, com a
   identidade RASGO (cores e tipografia), e os rótulos desenhados pelo
   programa (o leitor de SVG do Rack não desenha texto; a outra saída é
   converter o texto em curvas).
2. **Nome do plugin e dos módulos** ("Rasgo" como marca do plugin; os
   módulos com os mesmos nomes — FRONTEIRA, COMPARADOR… ficam como estão).
3. **Quais módulos primeiro.** Sugestão de piloto: 6 módulos que mostram o
   Rasgo sem depender de visor próprio — OSC, LADDER, ENVELOPE, RUNGLER,
   GENDYN, PRISM.
4. **Repositório separado** para as cascas (o instrumento fica como está).
5. **Baixar o SDK do Rack 2** (público, da VCV) para compilar o piloto aqui.

## 6. Plano em etapas

1. Gerador de cascas: do `Signal` de cada módulo (portas, parâmetros,
   `panel()`) para o `Module`/`ModuleWidget` do Rack, com as escalas de
   tensão por porta; e o gerador de painel SVG.
2. Piloto de 6 módulos com o SDK do Rack 2: compila, carrega, soa; teste de
   escala de tensão e de atraso.
3. Imagem dos painéis para o autor aprovar.
4. Os outros módulos em lotes, com os visores mais importantes reescritos.
5. Publicação na VCV Library; depois, a proposta ao Cardinal.

## 7. Riscos

- **Atraso por módulo** (um bloco curto): aceitável para a maioria; em
  cadeias longas de áudio ou realimentação curta, soa diferente do
  aplicativo. Medir no piloto e deixar o bloco configurável.
- **Custo de CPU** com bloco pequeno: a leitura de parâmetro por nome a cada
  chamada pesa; a casca deve passar os parâmetros por índice.
- **Manutenção:** cada módulo novo do Modular pede a casca (gerada) e, se
  tiver visor, o widget. O gerador reduz isso a pouco.
- **Expectativa:** no Rack não há seeds nem VARIA; a página do plugin deve
  dizer que o instrumento completo é o aplicativo.

## 8. O piloto (23/11/2026)

Autor: "pode baixar o kit vcv e começar o piloto" (o kit é gratuito, sem
cadastro: conferido na página da VCV antes do download).

- **Kit:** Rack SDK 2.6.6 (Linux x64) em `RASGO/third_party/vcv-rack-sdk-2.6.6/`,
  com sha256 e licença em `PROVENANCE.md`.
- **Onde:** `PROJETOS_AUTORAIS/RASGO_MODULAR_VCV/` (local, fora deste
  repositório; o repositório próprio é decisão do autor, §5.4). O DSP entra
  por cópia fixada do commit `bee38c0`, sem alteração.
- **A casca** (`src/Casca.hpp`): genérica, gerada do próprio módulo
  (parâmetros, portas, `panel()`); blocos de 16 amostras; ±5 V para áudio e
  CV, 0–10 V para gate/trigger, 1 V/oct igual.
- **Compila** sem aviso com o SDK (`-std=gnu++17` depois do `-std=c++11` do
  SDK; sem o `jq`, `make SLUG=RasgoModular VERSION=2.0.0`).
- **Teste sem janela** (`tests/teste_casca.cpp`, ligado à `libRack.so`): OSC a
  440 Hz exatos com pico de 5,00 V; +1 V na entrada de altura dobra a
  frequência; atraso de 16 amostras (0,33 ms); LADDER estável.
- **Achado do SDK:** o Rack compila plugins com `-funsafe-math-optimizations`;
  vigiar módulos que dependem de NaN/infinito ou de denormais (o Rasgo já
  zera denormais por conta própria).
- **No VCV Rack Free 2.6.6** (gratuito, `RASGO/third_party/vcv-rack-free-2.6.6/`,
  pasta de usuário isolada): os 6 módulos carregam e o modo de imagens do
  Rack (`-t`) desenha os painéis, montados das posições do `panel()`
  (`RASGO_MODULAR_VCV/docs/paineis_provisorios_2026-11-23.png`). Provisórios:
  visores vazios, rótulos dos jacks encostados no jack (o PJ301M do Rack é
  maior que o jack do Rasgo), rótulo do interruptor sobre ele.
- **Falta:** ouvir os módulos tocando no Rack (o autor); a aparência dos
  painéis e os visores (§5.1); nome do plugin (§5.2); repositório (§5.4).

## 9. O plugin completo (24/11/2026)

- **100 módulos** (todos menos SIGNAL-IN, POLY-IN, NOTE-OUT, MIXER, MASTER),
  gerados do catálogo por `RASGO_MODULAR_VCV/tools/gerar_plugin.py`; DSP em
  cópia fixada de `3dce50c`. Compila sem aviso; os 100 carregam no VCV Rack
  Free 2.6.6; o autor tocou vários ao vivo.
- **Visual decidido pelo autor:** B prata, faixa em degradê, reflexo nos
  knobs, arruela e círculo interno do jack em cinza, botões com luz laranja,
  visores na cor da família, wordmark RASGO na base (tamanho cheio, mesma
  altura em todos; à esquerda e com um parafuso só onde não cabe no
  centro), parafusos (topo só à direita), knobs em três tamanhos, painéis
  estreitados sem encolher objetos (7–17 HP; antes 7–22).
- **Informação:** dica do LEARN em inglês em cada knob e jack (o autor
  ofereceu esses textos também sob GPL), "User manual" para o guia do site,
  créditos no botão direito.
- **Falta para publicar:** repositório próprio e público (§5.4); conferir o
  `plugin.json` (sourceUrl do repositório novo); pedido à VCV Library (a VCV
  compila Windows/macOS/Linux); depois, a proposta ao Cardinal. Opcionais:
  visores próprios por módulo (hoje o traço genérico), polifonia por cópia,
  aproximar as fileiras curtas de KEY/TAP/SWITCH, o nome curto do COMPARADOR.

## 10. Repositório público (24/11/2026)

Autor: "pode fazer o repositório público rasgo-modular-vcv". Publicado em
https://github.com/lucioaraujo/rasgo-modular-vcv (commit `6f67a30`, DSP de
`66421f6`, com o VERSUS). Junto: as melhorias opcionais pedidas — visores
próprios (traço, XY para saídas x/y, pistas de gatilhos), polifonia até 16
vozes (SPACE mono), fileiras muito espaçadas aproximadas (KEY, TAP, SWITCH).
Próximo: o pedido à VCV Library (decisão do autor), depois o Cardinal. Pelo
padrão da família, a pasta `PROJETOS_AUTORAIS/RASGO_MODULAR_VCV/` passa a
ser a fonte única desse repositório.

## 11. Pedidos abertos (24/11/2026)

Autor: "pode publicar e fazer as solicitações".
- VCV Library: https://github.com/VCVRack/library/issues/1048 (commit `ebdee9e`;
  `make dist` testado no Linux; a VCV compila Windows/macOS/Linux).
- Cardinal: https://github.com/DISTRHO/Cardinal/issues/953.
Esperando a resposta dos mantenedores.

**Cardinal: recusado (resposta recebida em 24/11/2026).** O mantenedor
(dromer) fechou o #953: o projeto Cardinal não aceita código gerado com IA
e acha 100 módulos demais para um plugin; sugere, se quisermos, um fork
próprio do Cardinal com outro nome (permitido pela GPL). A recusa é de
política, não técnica. Recomendação: não fazer o fork por ora (custo alto,
ganho pequeno); o plugin segue no VCV Rack, e a VCV Library (#1048)
continua aberta.

**Decisão do autor (24/11/2026):** não fazer o "Rasgo Rack" (fork do
Cardinal) por ora. Aguardar a resposta da VCV Library (#1048); se ela também
recusar, publicar o `.vcvplugin` por conta própria (release no repositório
`rasgo-modular-vcv`, instalação manual).
