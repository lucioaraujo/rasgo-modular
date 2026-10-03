#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Gera as páginas de guia dos módulos, uma por idioma.

POR QUE É GERADO, e não escrito à mão: os 58 verbetes de módulo já existem
no catálogo do instrumento (`apps/panel/LearnCatalog.hpp`), traduzidos nos
quatro idiomas e cobertos por teste. Escrever uma segunda cópia no site
criaria duas versões do mesmo texto, que divergem na primeira correção — e
a que o público lê seria a que ninguém testa.

    ../build/rasgo_modular_learn_coverage --despejar-modulos > modulos.json
    python3 gerar_modulos.py

A moldura (fonte, faixa RASGO, cabeçalho, troca de idioma, contato) segue o
padrão dos outros sites da família — Antitotem e o portal RASGO —, e o
wordmark é LIDO de `index.html` em vez de copiado para cá, pelo mesmo
motivo do parágrafo acima.
"""
import html
import json
import pathlib
import re
import sys

AQUI = pathlib.Path(__file__).parent

# --- textos de moldura, por idioma ------------------------------------
# Os que já existiam nas páginas principais foram copiados de lá para
# manter uma voz só no site.
IDIOMAS = {
    "pt": {
        "arquivo": "modulos.html",
        "lang": "pt-BR",
        "faixa": "RASGO — família de instrumentos e pesquisa sonora",
        "pular": "Pular para o conteúdo",
        "idioma_label": "Idioma",
        "secoes_label": "Seções",
        "nav_conceito": "Conceito",
        "nav_modulos": "Módulos",
        "titulo": "Rasgo Modular — guia dos módulos",
        "descricao": "Os 58 módulos do Rasgo Modular explicados para quem está "
                     "começando: o que cada um faz com o som, o que muda em cada "
                     "controle e um exercício para fazer no app.",
        "pill": "58 módulos · 8 famílias",
        "h1": "Guia dos módulos",
        "lead": "Os 58 módulos do Rasgo Modular, família por família. Cada verbete "
                "conta o que o módulo faz com o som, como ele se relaciona com os "
                "vizinhos, o que muda em cada controle, e termina com alguns passos "
                "para você experimentar no app. Os exercícios partem de um patch em "
                "que o MIXER já está ligado ao MASTER, como em toda semente. Dentro "
                "do instrumento, a caixa LEARN mostra um resumo de cada módulo e de "
                "cada controle quando você passa o mouse sobre eles.",
        "indice": "Famílias",
        "n_modulos": "%d módulos",
        "rodape_1": "<strong>Rasgo Modular</strong> — Lúcio de Araújo, 2026. "
                    "Código sob GNU AGPL-3.0-or-later.",
        "rodape_2": "Os nomes de controles e portas citados aqui são conferidos contra "
                    "o código do instrumento.",
        "contato_h": "Contato",
        "contato_p": "Lúcio Araújo — autoria e desenvolvimento. Defeitos, "
                     "licença, crédito:",
        "dlg_fechar": "Fechar janela de contato",
        "dlg_titulo": "CONTATO",
        "dlg_copie": "Copie o endereço de e-mail:",
        "dlg_botao": "Copiar e-mail",
        "dlg_copiado": "E-mail copiado.",
        "dlg_fallback": "Selecione e copie o endereço acima.",
    },
    "en": {
        "arquivo": "modulos-en.html",
        "lang": "en",
        "faixa": "RASGO — family of instruments and sound research",
        "pular": "Skip to content",
        "idioma_label": "Language",
        "secoes_label": "Sections",
        "nav_conceito": "Concept",
        "nav_modulos": "Modules",
        "titulo": "Rasgo Modular — module guide",
        "descricao": "The 58 modules of Rasgo Modular explained for beginners: what each "
                     "one does to the sound, what every control changes, and an exercise "
                     "to try in the app.",
        "pill": "58 modules · 8 families",
        "h1": "Module guide",
        "lead": "The 58 modules of Rasgo Modular, family by family. Each entry "
                "tells you what the module does to the sound, how it relates to its "
                "neighbours, what each control changes, and ends with a few steps "
                "to try in the app. The exercises start from a patch where MIXER is "
                "already connected to MASTER, as in every seed. Inside the "
                "instrument, the LEARN box shows a summary of each module and each "
                "control when you hover over them.",
        "indice": "Families",
        "n_modulos": "%d modules",
        "rodape_1": "<strong>Rasgo Modular</strong> — Lúcio de Araújo, 2026. "
                    "Code under GNU AGPL-3.0-or-later.",
        "rodape_2": "The control and port names quoted here are checked against the "
                    "instrument's code.",
        "contato_h": "Contact",
        "contato_p": "Lúcio Araújo — authorship and development. Bugs, "
                     "licence, credit:",
        "dlg_fechar": "Close contact window",
        "dlg_titulo": "CONTACT",
        "dlg_copie": "Copy the email address:",
        "dlg_botao": "Copy email",
        "dlg_copiado": "Email copied.",
        "dlg_fallback": "Select and copy the address above.",
    },
    "fr": {
        "arquivo": "modulos-fr.html",
        "lang": "fr",
        "faixa": "RASGO — famille d'instruments et de recherche sonore",
        "pular": "Aller au contenu",
        "idioma_label": "Langue",
        "secoes_label": "Sections",
        "nav_conceito": "Concept",
        "nav_modulos": "Modules",
        "titulo": "Rasgo Modular — guide des modules",
        "descricao": "Les 58 modules du Rasgo Modular expliqués pour qui débute : ce que "
                     "chacun fait au son, ce que change chaque réglage, et un exercice à "
                     "faire dans l’application.",
        "pill": "58 modules · 8 familles",
        "h1": "Guide des modules",
        "lead": "Les 58 modules du Rasgo Modular, famille par famille. Chaque "
                "notice raconte ce que le module fait au son, comment il se situe "
                "parmi ses voisins, ce que change chaque réglage, et se termine par "
                "quelques étapes à essayer dans l’application. Les exercices "
                "partent d’un patch où le MIXER est déjà relié au MASTER, comme "
                "dans toutes les graines. Dans l’instrument, la boîte LEARN affiche "
                "un résumé de chaque module et de chaque réglage quand vous les "
                "survolez avec la souris.",
        "indice": "Familles",
        "n_modulos": "%d modules",
        "rodape_1": "<strong>Rasgo Modular</strong> — Lúcio de Araújo, 2026. "
                    "Code sous GNU AGPL-3.0-or-later.",
        "rodape_2": "Les noms de réglages et de prises cités ici sont vérifiés d’après "
                    "le code de l’instrument.",
        "contato_h": "Contact",
        "contato_p": "Lúcio Araújo — écriture et développement. Défauts, "
                     "licence, crédit :",
        "dlg_fechar": "Fermer la fenêtre de contact",
        "dlg_titulo": "CONTACT",
        "dlg_copie": "Copiez l’adresse e-mail :",
        "dlg_botao": "Copier l’e-mail",
        "dlg_copiado": "E-mail copié.",
        "dlg_fallback": "Sélectionnez et copiez l’adresse ci-dessus.",
    },
    "es": {
        "arquivo": "modulos-es.html",
        "lang": "es",
        "faixa": "RASGO — familia de instrumentos e investigación sonora",
        "pular": "Saltar al contenido",
        "idioma_label": "Idioma",
        "secoes_label": "Secciones",
        "nav_conceito": "Concepto",
        "nav_modulos": "Módulos",
        "titulo": "Rasgo Modular — guía de los módulos",
        "descricao": "Los 58 módulos del Rasgo Modular explicados para quien empieza: "
                     "qué hace cada uno con el sonido, qué cambia cada control y un "
                     "ejercicio para hacer en la aplicación.",
        "pill": "58 módulos · 8 familias",
        "h1": "Guía de los módulos",
        "lead": "Los 58 módulos del Rasgo Modular, familia por familia. Cada "
                "entrada cuenta qué hace el módulo con el sonido, cómo se relaciona "
                "con sus vecinos, qué cambia cada control, y termina con unos pasos "
                "para probar en la aplicación. Los ejercicios parten de un patch en "
                "el que el MIXER ya está conectado al MASTER, como en todas las "
                "semillas. Dentro del instrumento, la caja LEARN muestra un resumen "
                "de cada módulo y de cada control al pasar el ratón sobre ellos.",
        "indice": "Familias",
        "n_modulos": "%d módulos",
        "rodape_1": "<strong>Rasgo Modular</strong> — Lúcio de Araújo, 2026. "
                    "Código bajo GNU AGPL-3.0-or-later.",
        "rodape_2": "Los nombres de controles y conexiones citados aquí se verifican "
                    "contra el código del instrumento.",
        "contato_h": "Contacto",
        "contato_p": "Lúcio Araújo — autoría y desarrollo. Defectos, "
                     "licencia, crédito:",
        "dlg_fechar": "Cerrar ventana de contacto",
        "dlg_titulo": "CONTACTO",
        "dlg_copie": "Copie la dirección de correo:",
        "dlg_botao": "Copiar correo",
        "dlg_copiado": "Correo copiado.",
        "dlg_fallback": "Seleccione y copie la dirección de arriba.",
    },
}

# Rótulos dos três níveis, com as MESMAS palavras que o tutorial do
# instrumento usa ("rápido, como funciona, e um experimento") — quem leu o
# tutorial reconhece a estrutura na página.
NIVEIS = {
    "pt": ("Rápido", "Como funciona", "Experimente"),
    "en": ("Quick", "How it works", "Try this"),
    "fr": ("Rapide", "Comment ça marche", "À essayer"),
    "es": ("Rápido", "Cómo funciona", "Pruebe esto"),
}

# Títulos das quatro partes de um verbete do guia didático (ver
# `ESTILO.md`). No arquivo de cada verbete os títulos são sempre em
# português — são chaves —, e a página recebe o título do idioma.
PARTES = ("O que é", "Como pensar nele", "Controles", "Experimente")
TITULOS_PARTES = {
    "pt": ("O que é", "Como pensar nele", "Controles", "Experimente"),
    "en": ("What it is", "How to think about it", "Controls", "Try this"),
    "fr": ("Ce que c’est", "Comment l’aborder", "Commandes", "À essayer"),
    "es": ("Qué es", "Cómo pensarlo", "Controles", "Pruebe esto"),
}


def ler_verbete(codigo, tipo):
    """Lê `guia/<idioma>/<TIPO>.md`, o verbete didático (ver ESTILO.md).

    Devolve {parte: [linhas]} ou None se o arquivo não existe — e aí a
    página cai no texto curto do LEARN, como antes. Assim o guia pode
    crescer módulo a módulo sem nunca publicar um buraco.
    """
    arq = AQUI / "guia" / codigo / (tipo + ".md")
    if not arq.exists():
        return None
    partes, atual = {}, None
    for linha in arq.read_text(encoding="utf-8").splitlines():
        if linha.startswith("## "):
            atual = linha[3:].strip()
            if atual not in PARTES:
                sys.exit("%s: parte desconhecida '%s'" % (arq, atual))
            partes[atual] = []
        elif atual is not None:
            partes[atual].append(linha)
    falta = [p for p in PARTES if p not in partes]
    if falta:
        sys.exit("%s: faltam as partes %s" % (arq, falta))
    return partes


def inline(texto):
    """Escapa e aplica a única marcação permitida no verbete: `código`."""
    return re.sub(r"`([^`]+)`", r"<code>\1</code>", e(texto))


def html_verbete(codigo, partes):
    """As quatro partes em HTML: parágrafos, lista de controles, passos."""
    titulos = TITULOS_PARTES[codigo]
    L = []
    for chave, titulo in zip(PARTES, titulos):
        linhas = partes[chave]
        L.append('    <h4>%s</h4>' % e(titulo))
        paragrafo, lista, numerada = [], [], []

        def fecha_paragrafo():
            if paragrafo:
                L.append("    <p>%s</p>" % inline(" ".join(paragrafo)))
                paragrafo.clear()

        def fecha_listas():
            if lista:
                L.append('    <ul class="controles">')
                for item in lista:
                    nome, _, resto = item.partition(":")
                    if resto:
                        # o francês põe espaço fino inseparável antes dos
                        # dois-pontos ("FREQ : ..."); os outros idiomas, não
                        sep = "\u202f:" if codigo == "fr" else ":"
                        L.append("      <li><b>%s</b>%s%s</li>"
                                 % (e(nome.strip()), sep, inline(resto)))
                    else:
                        L.append("      <li>%s</li>" % inline(item))
                L.append("    </ul>")
                lista.clear()
            if numerada:
                L.append('    <ol class="passos">')
                for item in numerada:
                    L.append("      <li>%s</li>" % inline(item))
                L.append("    </ol>")
                numerada.clear()

        for linha in linhas + [""]:
            s = linha.strip()
            m = re.match(r"^\d+\.\s+(.*)", s)
            if s.startswith("- "):
                fecha_paragrafo()
                lista.append(s[2:])
            elif m:
                fecha_paragrafo()
                numerada.append(m.group(1))
            elif not s:
                fecha_paragrafo()
                fecha_listas()
            else:
                if lista or numerada:
                    fecha_listas()
                paragrafo.append(s)
    return L


# Painel de cada módulo, desenhado pelo próprio app
# (`RASGO_EXPORTAR_PAINEIS`, com fontes de teste ligadas para os displays
# mostrarem o módulo funcionando) — ver README.md.
ALT_PAINEL = {"pt": "Painel do módulo %s", "en": "%s module panel",
              "fr": "Panneau du module %s", "es": "Panel del módulo %s"}


def figura_painel(codigo, tipo):
    arq = AQUI / "assets" / "modulos" / (tipo + ".webp")
    if not arq.exists():
        return []
    try:
        from PIL import Image
        w, h = Image.open(arq).size
        dim = ' width="%d" height="%d"' % (w // 2, h // 2)
    except Exception:
        dim = ""
    return ['    <figure class="painel"><img src="assets/modulos/%s.webp" alt="%s"%s loading="lazy" /></figure>'
            % (tipo, e(ALT_PAINEL[codigo] % tipo), dim)]


PAGINA_PRINCIPAL = {"pt": "index.html", "en": "en.html",
                    "fr": "fr.html", "es": "es.html"}


def wordmark():
    """Lê o SVG do wordmark de `index.html` — uma cópia só no site."""
    fonte = (AQUI / "index.html").read_text(encoding="utf-8")
    m = re.search(r'(<svg class="wordmark".*?</svg>)', fonte, re.S)
    if not m:
        sys.exit("não achei o <svg class=\"wordmark\"> em index.html")
    return m.group(1)


def e(texto):
    return html.escape(texto, quote=False)


def pagina(codigo, dados, familias, marca):
    t = dados
    niveis = NIVEIS[codigo]
    L = []
    L.append("<!doctype html>")
    L.append('<html lang="%s">' % t["lang"])
    L.append("<head>")
    L.append('<meta charset="utf-8" />')
    L.append('<meta name="viewport" content="width=device-width, initial-scale=1" />')
    L.append("<title>%s</title>" % e(t["titulo"]))
    L.append('<meta name="description" content="%s" />' % html.escape(t["descricao"]))
    L.append('<link rel="icon" href="assets/identity/favicon.svg" type="image/svg+xml" />')
    L.append('<link rel="icon" href="assets/identity/favicon-32.png" type="image/png" sizes="32x32" />')
    L.append('<link rel="apple-touch-icon" href="assets/identity/favicon-256.png" />')
    L.append('<link rel="stylesheet" href="styles.css" />')
    L.append('<script src="assets/contact.js" defer></script>')
    L.append("</head>")
    L.append("<body>")
    L.append('<div class="rasgo-strip"><a href="https://rasgoinstruments.arquiviagem.net/">%s</a></div>'
             % e(t["faixa"]))
    L.append('<a class="skip-link" href="#conteudo">%s</a>' % e(t["pular"]))
    L.append("")
    L.append('<div class="site-header">')
    L.append('  <a class="brand" href="#top" aria-label="Rasgo Modular">')
    L.append(marca)
    L.append("    <span>MODULAR</span>")
    L.append("  </a>")
    L.append('  <div class="header-nav">')
    L.append('    <nav class="page-nav" aria-label="%s">' % e(t["secoes_label"]))
    L.append('      <a href="%s">%s</a>' % (PAGINA_PRINCIPAL[codigo], e(t["nav_conceito"])))
    L.append('      <a href="%s" aria-current="page">%s</a>' % (t["arquivo"], e(t["nav_modulos"])))
    L.append("    </nav>")
    L.append('    <nav class="lang-switch" aria-label="%s">' % e(t["idioma_label"]))
    for c in ("pt", "en", "fr", "es"):
        atual = ' aria-current="true"' if c == codigo else ""
        L.append('      <a href="%s"%s>%s</a>'
                 % (IDIOMAS[c]["arquivo"], atual, c.upper()))
    L.append("    </nav>")
    L.append("  </div>")
    L.append("</div>")
    L.append("")
    L.append('<main id="conteudo">')
    L.append('<section class="hero" id="top">')
    L.append('  <span class="status-pill">%s</span>' % e(t["pill"]))
    L.append("  <h1>%s</h1>" % e(t["h1"]))
    L.append('  <p class="lead">%s</p>' % e(t["lead"]))
    L.append("</section>")
    L.append("")
    # índice das famílias: 58 módulos numa página só precisam de atalho
    L.append('<nav class="familias" aria-label="%s">' % e(t["indice"]))
    for fam in familias:
        L.append('  <a href="#fam-%s">%s <span>%d</span></a>'
                 % (fam["familia"], e(fam["familia"]), len(fam["modulos"])))
    L.append("</nav>")
    L.append("")
    escuro = False
    for fam in familias:
        classe = "section section-dark" if escuro else "section"
        escuro = not escuro
        L.append('<section class="%s" id="fam-%s">' % (classe, fam["familia"]))
        L.append("  <h2>%s <small>%s</small></h2>"
                 % (e(fam["familia"]), e(t["n_modulos"] % len(fam["modulos"]))))
        for mod in fam["modulos"]:
            verbete = mod[codigo] or mod["pt"]
            L.append('  <article class="modulo" id="mod-%s">' % mod["tipo"])
            L.append("    <h3>%s</h3>" % e(mod["tipo"]))
            L.extend(figura_painel(codigo, mod["tipo"]))
            didatico = ler_verbete(codigo, mod["tipo"])
            if didatico is not None:
                L.extend(html_verbete(codigo, didatico))
                L.append("  </article>")
                continue
            for i, texto in enumerate(verbete):
                if not texto.strip():
                    continue
                L.append('    <p><span class="nivel">%s</span> %s</p>'
                         % (e(niveis[i]), e(texto)))
            L.append("  </article>")
        L.append("</section>")
    L.append("")
    L.append('<section class="section" id="contato">')
    L.append("  <h2>%s</h2>" % e(t["contato_h"]))
    L.append("  <p>%s</p>" % e(t["contato_p"]))
    L.append('  <div class="contact-row">')
    L.append('    <button type="button" class="button button-primary" data-contact-trigger>rasgo.instruments@gmail.com</button>')
    L.append('    <a class="button" href="https://github.com/lucioaraujo/rasgo-modular">GitHub</a>')
    L.append("  </div>")
    L.append("</section>")
    L.append("</main>")
    L.append("")
    L.append("<footer>")
    L.append("  <p>%s</p>" % t["rodape_1"])
    L.append("  <p>%s</p>" % e(t["rodape_2"]))
    L.append("</footer>")
    L.append('<dialog class="contact-dialog" data-contact-dialog aria-labelledby="contact-title">')
    L.append('  <button class="dialog-close" type="button" data-contact-close aria-label="%s">&times;</button>'
             % e(t["dlg_fechar"]))
    L.append('  <h2 id="contact-title">%s</h2>' % e(t["dlg_titulo"]))
    L.append("  <p>%s</p>" % e(t["dlg_copie"]))
    L.append("  <code data-contact-address></code>")
    L.append('  <button class="button button-primary" type="button" data-contact-copy>%s</button>'
             % e(t["dlg_botao"]))
    L.append('  <p class="contact-feedback" data-contact-feedback data-copied="%s" data-fallback="%s" aria-live="polite"></p>'
             % (html.escape(t["dlg_copiado"]), html.escape(t["dlg_fallback"])))
    L.append("</dialog>")
    L.append("</body>")
    L.append("</html>")
    return "\n".join(L) + "\n"


def main():
    familias = json.loads((AQUI / "modulos.json").read_text(encoding="utf-8"))
    marca = wordmark()
    total = sum(len(f["modulos"]) for f in familias)
    for codigo, dados in IDIOMAS.items():
        destino = AQUI / dados["arquivo"]
        destino.write_text(pagina(codigo, dados, familias, marca), encoding="utf-8")
        print("%-18s %d módulos em %d famílias"
              % (dados["arquivo"], total, len(familias)))


if __name__ == "__main__":
    main()
