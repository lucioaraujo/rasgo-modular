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
