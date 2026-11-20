# Rasgo Modular — licenças / licensing

[Português](#português) · [English](#english)

## Português

### Em uma frase

O **instrumento** Rasgo Modular é **AGPL-3.0-or-later**. Os **módulos de
DSP** (as pastas `src/dsp/` e `src/core/`) são oferecidos **também** sob
**GPL-3.0-or-later**: quem os recebe escolhe uma das duas.

Decisão do autor, Lúcio de Araújo, em 20/11/2026, para que os módulos
possam ser levados ao **VCV Rack** e ao **Cardinal** (que aceita só
GPL-3.0-or-later), sem mudar a licença do instrumento.

### O que vale para cada parte

| Parte | Pastas | Licença |
|---|---|---|
| Módulos de DSP e o núcleo do grafo | `src/dsp/`, `src/core/` | **AGPL-3.0-or-later ou GPL-3.0-or-later**, à escolha de quem recebe |
| O instrumento: aplicativo JUCE, plugin, painel, seed, VARIA, LEARN, interface | `apps/`, `src/ui/`, `src/io/`, `src/saida/` e o resto do repositório | **AGPL-3.0-or-later** |
| Site, guias e dossiês | `website/`, `guia/`, `dossies/` | os mesmos termos do repositório (AGPL-3.0-or-later) |
| Código de terceiros | ver abaixo | a licença de cada um |

Os textos das licenças: [`LICENSE`](LICENSE) (AGPL-3.0) e
[`LICENSE.GPL-3.0`](LICENSE.GPL-3.0) (GPL-3.0).

### Por que isso não cria problema

- **Autoria.** Todo o código de `src/dsp/` e `src/core/` é de Lúcio de
  Araújo (conferido no histórico do Git em 20/11/2026). Os trechos vindos
  de outros instrumentos da família (Rasgo Synth Studio, Asterismus,
  Antitotem, Aquorbium, Trioio, Batruc) são do mesmo autor, que pode
  oferecer uma segunda licença para o que é seu.
- **O código de outra pessoa já é GPL-3.0-or-later.** Dois arquivos trazem
  trechos do Navalha 2, de **Glerm Soares** e Lúcio Araújo, sob
  GPL-3.0-or-later: `src/dsp/PitchShift.hpp` (o algoritmo de Miller
  Puckette, domínio público, pela implementação do Navalha) e
  `src/dsp/Sampler.hpp` (o SlicePlayer). Essa licença é a mesma da oferta,
  então nada muda para esse código; o crédito a Glerm Soares fica em cada
  arquivo e em [`CREDITS_AND_SOURCES.md`](CREDITS_AND_SOURCES.md).
- **Leituras de outros projetos não entram como código.** Sapphire,
  ChowTape, CVfunk, Bidoo, Mutable Instruments e os demais citados nos
  dossiês foram lidos como conceito; nenhum código deles está nos módulos.
- **O JUCE fica de fora.** O JUCE é usado sob AGPL-3.0-only, só pelo
  aplicativo e pelo plugin (`apps/juce/`), que continuam AGPL. Os módulos
  não dependem dele.
- **O `dr_wav`** (domínio público ou MIT-0) fica em `src/io/`, fora dos
  módulos.

### O que muda na prática

- **Para quem usa o aplicativo ou o plugin:** nada. Continuam AGPL.
- **Para quem quiser portar os módulos para o VCV Rack ou o Cardinal:**
  pode usar `src/dsp/` e `src/core/` sob GPL-3.0-or-later, como o Cardinal
  exige, mantendo os avisos de autoria e os créditos.
- **A diferença entre as duas licenças:** a AGPL pede o código-fonte também
  de quem oferece o programa como serviço pela rede; a GPL, só de quem
  distribui o programa. Quem escolher a GPL para os módulos não tem essa
  obrigação de rede. É o único efeito da oferta, e é o que o Cardinal pede.
- **A oferta vale para as versões publicadas com ela** e não pode ser
  retirada delas. As versões futuras seguem a decisão do autor.
- **Contribuições de outras pessoas** em `src/dsp/` ou `src/core/` só são
  aceitas sob as duas licenças (AGPL-3.0-or-later e GPL-3.0-or-later), para
  a oferta continuar valendo.
- **Nome e marca.** As licenças cobrem o código. O nome "RASGO" e a marca
  não são licenciados por elas: um port deve deixar claro que é um port.

### Para portar (o que vai e o que fica)

Vão: o som de cada módulo (o DSP), as portas, os parâmetros, o desenho do
painel em milímetros (`panel()`). Ficam no instrumento: as seeds, o VARIA,
o MUDA, a relação e a condução dos cabos como processo do patch, o LEARN e a
interface. Plano em [`DESENVOLVIMENTO.md`](DESENVOLVIMENTO.md) ("Módulos
para VCV Rack / Cardinal").

## English

### In one sentence

The Rasgo Modular **instrument** is **AGPL-3.0-or-later**. Its **DSP
modules** (the `src/dsp/` and `src/core/` folders) are **also** offered
under **GPL-3.0-or-later**: whoever receives them chooses either one.

Decision by the author, Lúcio de Araújo, on 20 Nov 2026, so that the modules
can be brought to **VCV Rack** and **Cardinal** (which accepts only
GPL-3.0-or-later), without changing the instrument's license.

### What applies to each part

| Part | Folders | License |
|---|---|---|
| DSP modules and the graph core | `src/dsp/`, `src/core/` | **AGPL-3.0-or-later or GPL-3.0-or-later**, at the recipient's choice |
| The instrument: JUCE application, plugin, panel, seed, VARIA, LEARN, interface | `apps/`, `src/ui/`, `src/io/`, `src/saida/` and the rest of the repository | **AGPL-3.0-or-later** |
| Website, guides and dossiers | `website/`, `guia/`, `dossies/` | the repository's terms (AGPL-3.0-or-later) |
| Third-party code | see below | each one's own license |

License texts: [`LICENSE`](LICENSE) (AGPL-3.0) and
[`LICENSE.GPL-3.0`](LICENSE.GPL-3.0) (GPL-3.0).

### Why this is safe

- **Authorship.** All code in `src/dsp/` and `src/core/` is by Lúcio de
  Araújo (checked against the Git history on 20 Nov 2026). Code that came
  from other instruments of the family (Rasgo Synth Studio, Asterismus,
  Antitotem, Aquorbium, Trioio, Batruc) is by the same author, who can offer
  a second license for his own work.
- **Someone else's code is already GPL-3.0-or-later.** Two files carry code
  from Navalha 2, by **Glerm Soares** and Lúcio Araújo, under
  GPL-3.0-or-later: `src/dsp/PitchShift.hpp` (Miller Puckette's public-domain
  algorithm, through Navalha's implementation) and `src/dsp/Sampler.hpp`
  (the SlicePlayer). That is the very license being offered, so nothing
  changes for that code; the credit to Glerm Soares stays in each file and in
  [`CREDITS_AND_SOURCES.md`](CREDITS_AND_SOURCES.md).
- **Other projects were read, not copied.** Sapphire, ChowTape, CVfunk,
  Bidoo, Mutable Instruments and the others cited in the dossiers were read
  for concepts; none of their code is in the modules.
- **JUCE stays out.** JUCE is used under AGPL-3.0-only, by the application
  and the plugin only (`apps/juce/`), which remain AGPL. The modules do not
  depend on it.
- **`dr_wav`** (public domain or MIT-0) lives in `src/io/`, outside the
  modules.

### What this means in practice

- **For anyone using the application or the plugin:** nothing changes. They
  remain AGPL.
- **For anyone porting the modules to VCV Rack or Cardinal:** `src/dsp/` and
  `src/core/` may be used under GPL-3.0-or-later, as Cardinal requires,
  keeping the authorship notices and credits.
- **The difference between the two:** the AGPL also asks for the source code
  from whoever offers the program as a network service; the GPL only from
  whoever distributes it. Choosing the GPL for the modules removes that
  network obligation. That is the only effect of the offer, and it is what
  Cardinal asks for.
- **The offer applies to the versions published with it** and cannot be
  withdrawn from them. Future versions follow the author's decision.
- **Contributions from other people** to `src/dsp/` or `src/core/` are
  accepted only under both licenses (AGPL-3.0-or-later and
  GPL-3.0-or-later), so that the offer keeps holding.
- **Name and mark.** The licenses cover the code. The "RASGO" name and mark
  are not licensed by them: a port should say clearly that it is a port.

### Porting (what goes and what stays)

Goes: each module's sound (the DSP), its ports, its parameters, its panel
layout in millimetres (`panel()`). Stays with the instrument: seeds, VARIA,
MUDA, cable relation and conduction as a process of the patch, LEARN and the
interface. Plan in [`DESENVOLVIMENTO.md`](DESENVOLVIMENTO.md) ("Módulos
para VCV Rack / Cardinal").
