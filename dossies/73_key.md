# 73 — KEY (TOM): a tonalidade de quem toca

Data: 11/11/2026. Estado: **em implementação** (fila aprovada pelo autor,
"avance"). Origem: estudo da família §4.9 (Maravi, `key_detection.py`) e
§4.4b (Trioio, croma do grave); §4.12: nenhum módulo do Cardinal detecta
tonalidade.

## Portão conceitual

1. **Território.** Perfis tonais de C. Krumhansl e E. Kessler (1982):
   ouvintes avaliaram o quanto cada uma das 12 classes "pertence" a um
   contexto maior ou menor; correlacionar um histograma de alturas com os
   24 perfis estima a tonalidade (algoritmo Krumhansl–Schmuckler). No
   RASGO, o Maravi já usa isto para escrever a armadura de clave.
2. **Tensão.** O patch do Modular toca numa escala que a seed escolheu; o
   músico, ao cantar ou tocar por cima (SIGNAL-IN), está noutra. O TOM fecha
   o ciclo: o patch passa a responder no tom de quem toca.
3. **Unidade.** A **linha de alturas** de quem toca (classes de altura com
   a duração de cada nota).
4. **Percebe / conserva / transforma / recusa / esquece.** Percebe notas
   (PTCH + GATE). Conserva um histograma das 12 classes **ponderado pela
   duração** (a nota longa pesa mais que a de passagem — a intuição do
   Maravi). Transforma em tônica + modo + certeza. Recusa: timbre,
   oitava, acordes simultâneos (uma linha por vez). Esquece: o histograma
   desbota no tempo (MEM), para seguir modulações.
5. **Regra, recurso, risco, limite.** Regra: o tom só muda quando outro
   vence por ESTAB notas seguidas (histerese — não tremular entre relativos).
   Risco: ambiguidade real (relativa maior/menor, cromatismo); a resposta é
   honesta: a CERTEZA sai como CV.
6. **Consequência.** RAIZ e ESCALA vão ao QUANTIZER (a harmonia do patch
   segue quem toca); MUDOU pulsa a cada troca (gesto de forma); CERTEZA pode
   abrir/fechar algo (quanto mais claro o tom, mais o patch se afirma).
7. **Diferença no Modular.** O SCOPE mede a altura; o MARKOV aprende
   transições; nenhum infere tonalidade.
8. **Diferença para o Maravi.** Lá, análise offline revisada por gente,
   para escrever partitura. Aqui, **causal e ao vivo** — e o erro é material
   musical: um tom mal inferido é uma harmonia que responde torto.
9. **Escuta.** Êxito: cantar em Ré menor e ouvir o patch ir para Ré menor
   em poucas notas, sem tremular. Fracasso: troca a cada nota, ou nunca
   troca. Nominal: ninguém liga RAIZ/ESCALA — a receita didática precisa
   mostrar o cabo.
10. **Proveniência.** Krumhansl & Kessler (1982), perfis publicados; a
    ponderação por duração e a margem como medida de ambiguidade, do Maravi
    (AGPL, mesmo autor). Código próprio.

## Comportamento

- Entradas PTCH (1 V/oct), GATE (borda = nota; sem cabo, nota quando a
  classe muda), RST.
- A cada nota, a anterior soma sua **duração** à sua classe; o histograma
  desbota com constante de tempo MEM (2–60 s).
- Correlação de Pearson com os 24 perfis K–K → melhor e segunda melhor.
- **Histerese:** o tom publicado só muda quando o mesmo candidato vence por
  ESTAB notas seguidas (1–16).
- Saídas: RAIZ (tônica/12 V — é também a tônica em 1 V/oct), ESCALA (no
  formato do QUANTIZER: Maior = 1/11, menor natural = 2/11), CERTEZA
  (margem para o segundo, 0–1), MUDOU (pulso de 10 ms).

## Na seed

Entra no rack da seed (regra de 9/11). Papel dedicado depois da escuta.
