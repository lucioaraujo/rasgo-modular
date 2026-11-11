# SUPPLY — a alimentação variável

**Módulo 88** · **Família:** MODULATE
**Dossiê técnico:** [`../dossies/88_supply.md`](../dossies/88_supply.md)

---

## A ideia

Num circuito, quando a bateria enfraquece, tudo muda junto: o relógio
desacelera, o som perde corpo e os osciladores começam a desafinar uns dos
outros. O SUPPLY traz essa causa comum para o patch, **por cabos**:
- **ENERGIA** é a tensão;
- a **CARGA** (o som do patch) a faz ceder;
- **TRILHO** é o que sobra, para ligar onde quiser (nível, corte, andamento);
- **D1–D4** só aparecem com o trilho baixo, e fazem as vozes se
  desencontrarem;
- **FALHA** marca o apagão e a volta.

## Como cabear

```
MIXER (L+R) → SUPPLY (CARGA)
SUPPLY (TRILHO) → VCA (CV1)       o patch perde corpo quando soa forte
SUPPLY (D1) → OSC (1V/O), (D2) → outro OSC (1V/O)   e desafina
SUPPLY (FALHA) → APOCRIFO (CIT)   o apagão rasga o tempo
```

Cada controle e cada jack estão explicados no LEARN do app, nos quatro
idiomas.
