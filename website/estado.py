#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Troca as quatro páginas entre ANTES e DEPOIS da release.

    python3 estado.py --antes              # sem release: o estado de hoje
    python3 estado.py --depois 0.1.0       # com release: downloads por plataforma

Por que um script e não edição à mão: são duas regiões × quatro idiomas =
oito trechos que precisam mudar JUNTOS. No dia de publicar, editar isso à
mão é como se erra — uma página fica anunciando "ainda não há release" com
o resto do site já publicado, e ninguém nota porque ninguém lê as quatro
línguas. Aqui o estado é um comando e não uma lembrança.

As regiões são delimitadas no HTML por
`<!-- PILULA:INICIO -->`/`<!-- PILULA:FIM -->` e
`<!-- ESTADO:INICIO -->`/`<!-- ESTADO:FIM -->`. Fora delas o script não
toca em nada.

Os nomes dos arquivos de download são os do `CPACK_PACKAGE_FILE_NAME` do
`CMakeLists.txt` — nome determinístico justamente porque virou contrato
desta página. Se um mudar lá, muda aqui, e o teste
`test_site_download.py` reclama.
"""
import argparse
import io
import pathlib
import re
import sys

AQUI = pathlib.Path(__file__).parent
PAGINAS = {"pt": "index.html", "en": "en.html", "fr": "fr.html", "es": "es.html"}
REPO = "https://github.com/lucioaraujo/rasgo-modular"

# Os três pacotes, na ordem em que aparecem na página. `arquivo` recebe a
# versão por formatação — o mesmo padrão do CPack.
PACOTES = [
    ("linux",   "rasgo-modular-%s-linux-x86_64.deb"),
    ("windows", "rasgo-modular-%s-windows-x64.exe"),
    ("macos",   "rasgo-modular-%s-macos-universal.dmg"),
]

TEXTO = {
    "pt": {
        "pilula_antes": "Em desenvolvimento — ainda sem release",
        "pilula_depois": "v%s — disponível para Linux, Windows e macOS",
        "antes": """  <div class="note">
    <p><strong>Ainda não há release.</strong> O instrumento roda, é
      reproduzível a partir do código e a integração contínua já empacota os
      três instaladores — mas nenhum foi publicado ainda.</p>
    <p>Esta página está pronta e em espera: pela regra editorial da família
      RASGO, o site de um instrumento só vai ao ar junto com o próprio
      instrumento. Quem quiser rodar agora, compila a partir do código —
      as instruções por sistema estão no
      <a href="%(repo)s/blob/main/INSTALL.md">INSTALL.md</a>.</p>
  </div>""",
        "depois": """  <div class="note">
    <p><strong>v%(v)s disponível.</strong> Um instalador por sistema, gerado
      e empacotado pela integração contínua a partir da tag
      <code>v%(v)s</code>.</p>
  </div>

  <div class="contact-row">
    <a class="button button-primary" href="%(repo)s/releases/download/v%(v)s/%(linux)s">Linux &middot; .deb</a>
    <a class="button" href="%(repo)s/releases/download/v%(v)s/%(windows)s">Windows &middot; .exe</a>
    <a class="button" href="%(repo)s/releases/download/v%(v)s/%(macos)s">macOS &middot; .dmg</a>
  </div>

  <p>Todas as versões, inclusive as de teste: <a href="%(repo)s/releases">página de releases no GitHub</a>.</p>

  <p><strong>Antes de baixar, duas coisas ditas por inteiro.</strong> O
    pacote Windows e o pacote macOS foram construídos, empacotados e
    testados <em>somente pela integração contínua</em> — o autor nunca abriu
    o aplicativo nesses dois sistemas. E o <code>.dmg</code> tem assinatura
    ad-hoc, sem Developer ID nem notarização: o Gatekeeper mostra aviso de
    desenvolvedor não identificado na primeira abertura.</p>

  <p><strong>Na primeira abertura, o Windows e o macOS mostram um aviso de
    segurança.</strong> O programa é livre e não tem certificado pago de
    assinatura; o aviso não indica defeito. O que clicar em cada sistema
    está no <a href="%(repo)s/blob/main/INSTALL.md#instalar-passo-a-passo">passo
    a passo da instalação</a>.</p>

  <p>Instalação, requisitos e como compilar a partir do código estão no
    <a href="%(repo)s/blob/v%(v)s/INSTALL.md">INSTALL.md</a>; o que mudou,
    no <a href="%(repo)s/blob/v%(v)s/CHANGELOG.md">CHANGELOG.md</a>.</p>""",
    },
    "en": {
        "pilula_antes": "In development — no release yet",
        "pilula_depois": "v%s — available for Linux, Windows and macOS",
        "antes": """  <div class="note">
    <p><strong>There is no release yet.</strong> The instrument runs, is
      reproducible from source, and continuous integration already packages
      all three installers — but none has been published yet.</p>
    <p>This page is finished and waiting: by the RASGO family's editorial
      rule, an instrument's site goes live together with the instrument
      itself. To run it now, build from source — the per-system instructions
      are in <a href="%(repo)s/blob/main/INSTALL.md">INSTALL.md</a>.</p>
  </div>""",
        "depois": """  <div class="note">
    <p><strong>v%(v)s is available.</strong> One installer per system, built
      and packaged by continuous integration from the <code>v%(v)s</code>
      tag.</p>
  </div>

  <div class="contact-row">
    <a class="button button-primary" href="%(repo)s/releases/download/v%(v)s/%(linux)s">Linux &middot; .deb</a>
    <a class="button" href="%(repo)s/releases/download/v%(v)s/%(windows)s">Windows &middot; .exe</a>
    <a class="button" href="%(repo)s/releases/download/v%(v)s/%(macos)s">macOS &middot; .dmg</a>
  </div>

  <p>All versions, including test builds: <a href="%(repo)s/releases">releases page on GitHub</a>.</p>

  <p><strong>Two things said in full before you download.</strong> The
    Windows and macOS packages were built, packaged and tested <em>by
    continuous integration only</em> — the author has never opened the
    application on either system. And the <code>.dmg</code> is ad-hoc
    signed, with no Developer ID and no notarisation: Gatekeeper will warn
    about an unidentified developer on first launch.</p>

  <p><strong>On first launch, Windows and macOS show a security
    warning.</strong> The program is free software without a paid
    code-signing certificate; the warning does not mean anything is wrong.
    What to click on each system is in the
    <a href="%(repo)s/blob/main/INSTALL.md#installing-step-by-step">step-by-step
    installation guide</a>.</p>

  <p>Installation, requirements and how to build from source are in
    <a href="%(repo)s/blob/v%(v)s/INSTALL.md">INSTALL.md</a>; what changed is
    in <a href="%(repo)s/blob/v%(v)s/CHANGELOG.md">CHANGELOG.md</a>.</p>""",
    },
    "fr": {
        "pilula_antes": "En développement — pas encore de release",
        "pilula_depois": "v%s — disponible pour Linux, Windows et macOS",
        "antes": """  <div class="note">
    <p><strong>Il n'y a pas encore de release.</strong> L'instrument
      fonctionne, il est reproductible depuis les sources, et l'intégration
      continue empaquette déjà les trois installateurs — mais aucun n'a
      encore été publié.</p>
    <p>Cette page est prête et en attente : selon la règle éditoriale de la
      famille RASGO, le site d'un instrument est mis en ligne en même temps
      que l'instrument lui-même. Pour l'utiliser dès maintenant, compilez
      depuis les sources — les instructions par système sont dans
      <a href="%(repo)s/blob/main/INSTALL.md">INSTALL.md</a>.</p>
  </div>""",
        "depois": """  <div class="note">
    <p><strong>v%(v)s disponible.</strong> Un installateur par système,
      construit et empaqueté par l'intégration continue depuis le tag
      <code>v%(v)s</code>.</p>
  </div>

  <div class="contact-row">
    <a class="button button-primary" href="%(repo)s/releases/download/v%(v)s/%(linux)s">Linux &middot; .deb</a>
    <a class="button" href="%(repo)s/releases/download/v%(v)s/%(windows)s">Windows &middot; .exe</a>
    <a class="button" href="%(repo)s/releases/download/v%(v)s/%(macos)s">macOS &middot; .dmg</a>
  </div>

  <p>Toutes les versions, y compris celles de test : <a href="%(repo)s/releases">page des releases sur GitHub</a>.</p>

  <p><strong>Deux choses dites en entier avant de télécharger.</strong> Les
    paquets Windows et macOS ont été construits, empaquetés et testés
    <em>uniquement par l'intégration continue</em> — l'auteur n'a jamais
    ouvert l'application sur ces deux systèmes. Et le <code>.dmg</code> est
    signé en ad-hoc, sans Developer ID ni notarisation : Gatekeeper
    affichera un avertissement de développeur non identifié au premier
    lancement.</p>

  <p><strong>Au premier lancement, Windows et macOS affichent un
    avertissement de sécurité.</strong> Le programme est libre, sans
    certificat de signature payant ; l'avertissement ne signale aucun
    défaut. Où cliquer sur chaque système : le
    <a href="%(repo)s/blob/main/INSTALL.md#installing-step-by-step">guide
    d'installation pas à pas</a> (en anglais et en portugais).</p>

  <p>Installation, prérequis et compilation depuis les sources sont dans
    <a href="%(repo)s/blob/v%(v)s/INSTALL.md">INSTALL.md</a> ; ce qui a
    changé, dans
    <a href="%(repo)s/blob/v%(v)s/CHANGELOG.md">CHANGELOG.md</a>.</p>""",
    },
    "es": {
        "pilula_antes": "En desarrollo — todavía sin release",
        "pilula_depois": "v%s — disponible para Linux, Windows y macOS",
        "antes": """  <div class="note">
    <p><strong>Todavía no hay release.</strong> El instrumento funciona, es
      reproducible desde el código y la integración continua ya empaqueta los
      tres instaladores — pero ninguno se ha publicado aún.</p>
    <p>Esta página está lista y en espera: por la regla editorial de la
      familia RASGO, el sitio de un instrumento se publica junto con el
      propio instrumento. Para usarlo ya, compile desde el código — las
      instrucciones por sistema están en
      <a href="%(repo)s/blob/main/INSTALL.md">INSTALL.md</a>.</p>
  </div>""",
        "depois": """  <div class="note">
    <p><strong>v%(v)s disponible.</strong> Un instalador por sistema,
      construido y empaquetado por la integración continua desde la etiqueta
      <code>v%(v)s</code>.</p>
  </div>

  <div class="contact-row">
    <a class="button button-primary" href="%(repo)s/releases/download/v%(v)s/%(linux)s">Linux &middot; .deb</a>
    <a class="button" href="%(repo)s/releases/download/v%(v)s/%(windows)s">Windows &middot; .exe</a>
    <a class="button" href="%(repo)s/releases/download/v%(v)s/%(macos)s">macOS &middot; .dmg</a>
  </div>

  <p>Todas las versiones, incluidas las de prueba: <a href="%(repo)s/releases">página de releases en GitHub</a>.</p>

  <p><strong>Dos cosas dichas por entero antes de descargar.</strong> Los
    paquetes de Windows y de macOS fueron construidos, empaquetados y
    probados <em>sólo por la integración continua</em> — el autor nunca abrió
    la aplicación en esos dos sistemas. Y el <code>.dmg</code> tiene firma
    ad-hoc, sin Developer ID ni notarización: Gatekeeper mostrará un aviso
    de desarrollador no identificado en la primera apertura.</p>

  <p><strong>En la primera apertura, Windows y macOS muestran un aviso de
    seguridad.</strong> El programa es libre y no tiene certificado de firma
    de pago; el aviso no indica ningún defecto. Qué pulsar en cada sistema:
    la <a href="%(repo)s/blob/main/INSTALL.md#instalar-passo-a-passo">guía de
    instalación paso a paso</a> (en portugués y en inglés).</p>

  <p>Instalación, requisitos y cómo compilar desde el código están en
    <a href="%(repo)s/blob/v%(v)s/INSTALL.md">INSTALL.md</a>; lo que cambió,
    en <a href="%(repo)s/blob/v%(v)s/CHANGELOG.md">CHANGELOG.md</a>.</p>""",
    },
}


def trocar(texto, marcador, novo):
    """Substitui o que está entre `<!-- X:INICIO -->` e `<!-- X:FIM -->`."""
    padrao = re.compile(
        r"(<!-- %s:INICIO -->\n).*?(  <!-- %s:FIM -->\n)" % (marcador, marcador),
        re.S)
    achou = padrao.search(texto)
    if not achou:
        sys.exit("não achei a região %s — os marcadores foram removidos?" % marcador)
    return padrao.sub(lambda m: m.group(1) + novo.rstrip("\n") + "\n" + m.group(2),
                      texto, count=1)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    g = ap.add_mutually_exclusive_group(required=True)
    g.add_argument("--antes", action="store_true",
                   help="estado sem release (o de hoje)")
    g.add_argument("--depois", metavar="VERSAO",
                   help="estado com release, p. ex. 0.1.0")
    args = ap.parse_args()

    if args.depois and not re.fullmatch(r"\d+\.\d+\.\d+", args.depois):
        sys.exit("versão precisa ser X.Y.Z (recebi %r)" % args.depois)

    for codigo, arquivo in PAGINAS.items():
        caminho = AQUI / arquivo
        s = caminho.read_text(encoding="utf-8")
        t = TEXTO[codigo]
        if args.antes:
            pilula = t["pilula_antes"]
            bloco = t["antes"] % {"repo": REPO}
        else:
            v = args.depois
            pilula = t["pilula_depois"] % v
            dados = {"v": v, "repo": REPO}
            for chave, molde in PACOTES:
                dados[chave] = molde % v
            bloco = t["depois"] % dados
        s = trocar(s, "PILULA",
                   '  <span class="status-pill">%s</span>' % pilula)
        s = trocar(s, "ESTADO", bloco)
        caminho.write_text(s, encoding="utf-8")
        print("%-12s %s" % (arquivo, "antes (sem release)" if args.antes
                            else "depois (v%s, com download)" % args.depois))


if __name__ == "__main__":
    main()
