# CUT-UP (CUTUP) — o recorte do Navalha num buffer ao vivo

**Módulo 85** · **Família:** SPACE
**Dossiê técnico:** [`../dossies/85_cutup.md`](../dossies/85_cutup.md)

---

## A ideia

STUT, BURST, MICRO e GAP sobre os últimos tempos do relógio, num buffer de 4 s. Os gestos vêm por gate, ou sozinhos com CHANCE. Bordas de 3 ms em cada fragmento e no gesto inteiro.

- **STUT:** 4 × tempo/4, limitado a 14–160 ms.
- **BURST:** 8 fatias dos últimos tempos, por saco embaralhado, em tempo/8 limitado a 18–72 ms; só os tempos que cabem no buffer.
- **MICRO:** 2–8×.
- **GAP:** um tempo de silêncio.

Cada controle e cada jack estão explicados no LEARN do app, nos quatro
idiomas.
