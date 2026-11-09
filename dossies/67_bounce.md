# BOUNCE — bolas que quicam

Implementação local de 9/11/2026, aprovada pelo autor a partir do estudo do
Cardinal ([`ESTUDO_cardinal_catalogo_e_licencas.md`](ESTUDO_cardinal_catalogo_e_licencas.md) §4b).
Fonte: [Bounce.hpp](../src/dsp/Bounce.hpp). Família MODULATE; 12 HP.
Licença autoral: AGPL-3.0-or-later. **Fora do catálogo da seed** (entra pela
BIBLIOTECA), como LISTEN, PHRASE e ARP.

## Contrato

Até 3 bolas em queda livre (gravidade GRAV, em "alturas por s²"); no chão, a
velocidade inverte e é multiplicada pela restituição ELAST. Cada intervalo entre
quiques encolhe pelo fator ELAST (o acelerando natural); a 1ª queda de altura 1
dura √(2/g).

**Entradas.** DROP (trig: relança), GRAV (CV: gravidade × 2^(2·cv), ±2).

**Saídas.** GT (pulso de 10 ms a cada quique de qualquer bola), ACC (força do
último quique, |v|/√(2g), mantida), H1–H3 (altura de cada bola, 0–1).

**Lançamento.** Bola 1 em 1; as outras em 1 − SPRD·b/3 menos até 0,1 de acaso
semeado. Uma bola para quando o pico seguinte seria menor que 10⁻⁴.

**Desvios Rasgo.** (a) WIND: cada quique multiplica a restituição por
1 ± 0,25·WIND (semeado); (b) AUTO: com todas paradas, relança — toca sozinho
para sempre.

## Testes

`tests/test_bounce.cpp`: 1ª queda 0,45175 s contra √(2/9,8) = 0,451754 s;
razão entre intervalos = ELAST (±0,03); acento decrescente; sem AUTO a bola para;
com AUTO segue quicando depois de 15 s e é determinístico; GRAV +0,5 → queda √2
mais rápida; DROP relança; robustez.

## Proveniência e limites

Mecânica de livro-texto (queda livre, colisão parcialmente elástica). Conceito
lido, sem cópia de código, no JW-Modules Bouncy Balls (BSD-3-Clause) e no
voxglitch Hazumi (GPL-3.0+), dentro do Cardinal 26.02.

As bolas não colidem entre si; sem paredes laterais (só altura).
