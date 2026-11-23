# Banco de escalas do Trioio — proveniência

**Arquivo:** `arquivo_trioio.txt` (2576 escalas, texto UTF-8, ~1,2 MB),
cópia **fixada e sem alteração**.
**Projeto de origem:** Trioio (família RASGO), `TRIOIO/data/escalas/arquivo_trioio.txt`,
gerado por `TRIOIO/tools/gerar_arquivo_de_escalas.py`.
**Commit de origem:** `7f09d60` (último commit que tocou o arquivo).
**sha256:** `1d26d19e1ab309402409997ec4e50216c723227b970e566331a26244cb01e529`
**Copiado:** 2026-11-23, pedido do autor (21/11: "depois vamos implementar o
banco completo"; 23/11: opção A da `dossies/PROPOSTA_banco_de_escalas.md`).
**Autoria:** Lúcio de Araújo (Trioio). Conteúdo próprio, escrito a partir da
teoria pública; cada escala cita a sua fonte. Nenhum arquivo de terceiros.

## Licença

AGPL-3.0-or-later, a do Trioio e a padrão da família. O banco e o seu
leitor (`src/escalas/`) ficam **só sob AGPL-3.0-or-later** — não entram na
oferta dupla GPL de `src/dsp` e `src/core` (`LICENSING.md`).

## Uso no Rasgo Modular

`tools/gerar_banco_escalas.py` lê esta cópia e gera
`src/escalas/BancoEscalasDados.inc` (nome, família, período, graus em cents e
fonte de cada escala; sem os campos de comportamento). Para atualizar:
copiar a versão nova do Trioio, registrar commit e sha256 aqui, rodar o
gerador. O patch guarda a escala por um número derivado do **nome**, então
reordenar o banco não troca a escala de um patch salvo.

## Alterações locais

Nenhuma no arquivo. A compactação acontece só no `.inc` gerado.
