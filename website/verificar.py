#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Confere o site: estrutura das páginas e os nomes dos downloads.

    python3 verificar.py

Duas verificações, e a segunda é a que justifica o arquivo:

1. **Estrutura das oito páginas** — marcação bem-formada, todo `href`/`src`
   interno apontando para arquivo que existe, e nenhum recurso externo além
   do portal e do GitHub.

2. **Os nomes dos downloads batem com o empacotamento.** A página publica
   links diretos para os arquivos da release, então esses nomes são
   CONTRATO entre o site e o `CPACK_PACKAGE_FILE_NAME` do `CMakeLists.txt`.
   Um `project(... VERSION ...)` que sobe para 0.2.0 sem que o site saiba
   deixaria três botões de download apontando para o vazio — e ninguém
   descobre isso lendo o HTML, só clicando depois de publicado.

   Aqui a versão do `CMakeLists.txt` é comparada com a do `estado.py` e,
   quando existe `build/CPackConfig.cmake`, com o nome que o CPack de fato
   gerou nesta máquina.

Não abre navegador: continua valendo que nenhum navegador real viu estas
páginas (ver README).
"""
import glob
import html.parser
import os
import pathlib
import re
import subprocess
import sys

AQUI = pathlib.Path(__file__).parent
RAIZ = AQUI.parent

VAZIAS = {"meta", "link", "img", "br", "hr", "source", "input", "area",
          "base", "col"}
# O próprio endereço público aparece nos <link rel="canonical|alternate"> do
# seo.py: são declarações de endereço, não recursos carregados.
EXTERNOS_OK = ("rasgoinstruments.arquiviagem.net", "github.com/lucioaraujo",
               "lucioaraujo.github.io/rasgo-modular/")

problemas = []


class Marcacao(html.parser.HTMLParser):
    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.pilha = []
        self.erros = []

    def handle_starttag(self, tag, attrs):
        if tag not in VAZIAS:
            self.pilha.append(tag)

    def handle_endtag(self, tag):
        if tag in VAZIAS:
            return
        if not self.pilha:
            self.erros.append("fecha </%s> sem abrir" % tag)
            return
        if self.pilha[-1] != tag:
            self.erros.append("fecha </%s> mas o aberto é <%s>"
                              % (tag, self.pilha[-1]))
        else:
            self.pilha.pop()


def conferir_estrutura():
    print("estrutura das páginas")
    for arq in sorted(glob.glob(str(AQUI / "*.html"))):
        nome = os.path.basename(arq)
        s = open(arq, encoding="utf-8").read()
        m = Marcacao()
        m.feed(s)
        if m.pilha:
            m.erros.append("não fechadas: %s" % m.pilha)
        for href in re.findall(r'href="([^"#h][^"]*)"', s):
            alvo = href.split("#")[0]
            if alvo and not (AQUI / alvo).exists():
                m.erros.append("link quebrado: %s" % alvo)
        for atr in re.findall(r'(?:src|srcset)="([^"]*)"', s):
            for parte in atr.split(","):
                alvo = parte.strip().split(" ")[0]
                if alvo and not alvo.startswith("http") \
                        and not (AQUI / alvo).exists():
                    m.erros.append("recurso ausente: %s" % alvo)
        for url in re.findall(r'(?:src|href)="(https?://[^"]*)"', s):
            if not any(ok in url for ok in EXTERNOS_OK):
                m.erros.append("recurso externo inesperado: %s" % url)
        print("  %-18s %s" % (nome, "ok" if not m.erros else "PROBLEMA"))
        for e in m.erros:
            print("      " + e)
            problemas.append("%s: %s" % (nome, e))


def versao_do_cmake():
    s = (RAIZ / "CMakeLists.txt").read_text(encoding="utf-8")
    m = re.search(r"project\([^)]*VERSION\s+(\d+\.\d+\.\d+)", s)
    if not m:
        problemas.append("não achei VERSION no project() do CMakeLists.txt")
        return None
    return m.group(1)


def conferir_downloads():
    print("\nnomes dos downloads contra o empacotamento")
    versao = versao_do_cmake()
    if versao is None:
        return
    print("  CMakeLists.txt project() VERSION: %s" % versao)

    estado = (AQUI / "estado.py").read_text(encoding="utf-8")
    moldes = re.findall(r'"(rasgo-modular-%s-[^"]+)"', estado)
    if len(moldes) != 3:
        problemas.append("esperava 3 moldes de pacote em estado.py, achei %d"
                         % len(moldes))
        return
    for molde in moldes:
        print("  molde: %s" % (molde % versao))

    # O nome que o CPack gerou de verdade nesta máquina, quando disponível.
    cfg = RAIZ / "build" / "CPackConfig.cmake"
    if cfg.exists():
        s = cfg.read_text(encoding="utf-8")
        m = re.search(r'set\(CPACK_PACKAGE_FILE_NAME "([^"]+)"\)', s)
        if m:
            real = m.group(1)
            print("  CPack nesta máquina: %s" % real)
            esperado = [(molde % versao).rsplit(".", 1)[0] for molde in moldes]
            if real not in esperado:
                problemas.append(
                    "CPack gera %r, que não está entre os nomes do site %r"
                    % (real, esperado))
        else:
            print("  (CPackConfig.cmake sem CPACK_PACKAGE_FILE_NAME)")
    else:
        print("  (sem build/CPackConfig.cmake — configure o build para "
              "conferir o nome real)")

    # a versão precisa aparecer formatada, não fixa no HTML
    for arq in ("index.html", "en.html", "fr.html", "es.html"):
        s = (AQUI / arq).read_text(encoding="utf-8")
        for achado in re.findall(r"rasgo-modular-(\d+\.\d+\.\d+)-", s):
            if achado != versao:
                problemas.append(
                    "%s aponta para a versão %s, mas o projeto está em %s"
                    % (arq, achado, versao))


def conferir_seo():
    """O bloco de seo.py em dia em todas as páginas, e a versão do JSON-LD
    igual à do CMake. gerar_modulos.py reescreve as páginas de módulos
    inteiras e apaga o bloco — sem esta conferência, isso passaria calado."""
    r = subprocess.run([sys.executable, str(AQUI / "seo.py"), "--verificar"],
                       capture_output=True, text=True)
    print(r.stdout.strip())
    if r.returncode != 0:
        problemas.append("metadados desatualizados: " + (r.stdout.strip() or r.stderr.strip()))
    m = re.search(r'^VERSAO = "([^"]+)"', (AQUI / "seo.py").read_text(encoding="utf-8"), re.M)
    cm = versao_do_cmake()
    if m and cm and m.group(1) != cm:
        problemas.append("seo.py declara VERSAO %s; o CMake diz %s" % (m.group(1), cm))


def main():
    conferir_estrutura()
    conferir_downloads()
    conferir_seo()
    print()
    if problemas:
        print("%d problema(s):" % len(problemas))
        for p in problemas:
            print("  " + p)
        return 1
    print("site conferido: sem problemas.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
