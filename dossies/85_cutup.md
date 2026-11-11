# 85 — CUT-UP (CUTUP): o recorte do Navalha num buffer ao vivo

Data: 11/11/2026. Estado: **em implementação** (fila aprovada pelo autor;
"avance em todos eles"). Origem: o `FragmentGesture` do Navalha 2 (Glerm Soares / Lúcio Araújo, GPL-3.0-or-later): os números, não o código — mesmo autor ou
coautoria creditada; conceito, não código.

## O que é

STUT, BURST, MICRO e GAP sobre os últimos tempos do relógio, num buffer de 4 s. Os gestos vêm por gate, ou sozinhos com CHANCE. Bordas de 3 ms em cada fragmento e no gesto inteiro.

## Comportamento

- **STUT:** 4 × tempo/4, limitado a 14–160 ms.
- **BURST:** 8 fatias dos últimos tempos, por saco embaralhado, em tempo/8 limitado a 18–72 ms; só os tempos que cabem no buffer.
- **MICRO:** 2–8×.
- **GAP:** um tempo de silêncio.

## Teste (`tests/test_cutup.cpp`)

- STUT ocupa exatamente um tempo (500 ms a 120 BPM);
- GAP fica em silêncio, com o maior salto em 0,0017;
- BURST pega fatias de vários tempos atrás.

## Na seed

Entra no rack da seed (regra de 9/11) na família SPACE. Um papel dedicado
fica para depois da escuta do autor.
