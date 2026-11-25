// Os visores (`Display`) dos módulos no navegador (25/11/2026, F3 do rack:
// ver dossies/ESTUDO_outras_plataformas.md). Tradução de `paintDisplay` do
// app JUCE (apps/juce/RasgoModularApp.cpp): o mesmo gráfico por tipo, as
// mesmas proporções e cores. Os dados vêm do motor (`rw_visor`, a mesma
// telemetria de `Rack::feedScopes`), num bloco de VISOR números:
//   [0, 220)    o anel da onda, em ordem cronológica
//   [220, 732)  as 4 lanes, 128 cada, cronológicas
//   [732, 748)  extras por tipo (ver `rw_visor` em rasgo_web.cpp)

export const VISOR = 220 + 4 * 128 + 16;
const NL = 128, NS = 220;
const NOTAS = ['C', 'C#', 'D', 'Eb', 'E', 'F', 'F#', 'G', 'Ab', 'A', 'Bb', 'B'];
const clamp = (v, a, b) => Math.min(b, Math.max(a, v));
const fonte = (px, negrito) => `${negrito ? 'bold ' : ''}${px * 0.86}px system-ui, sans-serif`;

function comAlfa(cor, a) {
  if (cor.startsWith('hsla(')) return cor.replace(/,\s*[\d.]+\)$/, `, ${a})`);
  if (cor.startsWith('#') && cor.length === 7) {
    const n = parseInt(cor.slice(1), 16);
    return `rgba(${n >> 16}, ${(n >> 8) & 255}, ${n & 255}, ${a})`;
  }
  return cor;
}

// banco de Goertzel logarítmico (`ui::scopeSpectrum`), com a taxa que o
// app usa (24000, a ressalva está em src/ui/ScopeTrace.hpp)
function espectro(v, nb) {
  const mag = new Float32Array(nb);
  for (let b = 0; b < nb; b++) {
    const f = 60 * Math.pow(200, b / (nb - 1));
    const cr = Math.cos(2 * Math.PI * f / 24000);
    let s1 = 0, s2 = 0;
    for (let k = 0; k < NS; k++) { const s0 = v[k] + 2 * cr * s1 - s2; s2 = s1; s1 = s0; }
    mag[b] = Math.sqrt(Math.abs(s1 * s1 + s2 * s2 - 2 * cr * s1 * s2)) / NS;
  }
  return mag;
}

// `c`: { T, wx, wy, dw, dh, tipo, v, traco, rotulo, espectro (SCOPE),
//        clip (MASTER: até quando o aviso fica aceso), escala (QUANTIZER) }
export function desenharVisor(g, c) {
  const { T, wx, wy, dw, dh, tipo, v, traco } = c;
  let rotulo = c.rotulo;
  const lane = (L, k) => v[NS + L * NL + k];
  const ex = (i) => v[NS + 4 * NL + i];
  const hline = (y, x0, x1) => g.fillRect(x0, y, x1 - x0, 1);
  const linha = (pts, cor, larg) => {
    g.strokeStyle = cor; g.lineWidth = larg; g.beginPath();
    for (let k = 0; k < pts.length; k += 2) (k ? g.lineTo : g.moveTo).call(g, pts[k], pts[k + 1]);
    g.stroke();
  };
  // sem dados ainda (a telemetria chega no quadro seguinte): só a moldura
  let pico = 0;
  if (v) for (let k = 0; k < NS; k++) pico = Math.max(pico, Math.abs(v[k]));

  if (v && dw > 6) {
    if (tipo === 'MASTER') {
      // VU com clip-latch: o pico contra −1 dBFS; o aviso acende quando o
      // limitador segurou algo e fica ~2 s
      const frac = Math.min(1, pico / 0.891);
      const barW = Math.floor(frac * (dw - 2));
      if (barW > 0) { g.fillStyle = frac < 0.7 ? T.prata : T.warning; g.fillRect(wx + 1, wy + 1, barW, dh - 2); }
      if (c.clip) { g.fillStyle = T.warning; g.fillRect(wx + dw - 7, wy + 1, 6, dh - 2); }
      rotulo = Math.round(20 * Math.log10(Math.max(pico, 1e-4))) + 'dB';
    } else if (tipo === 'LISTEN') {
      const row = Math.max(1, Math.floor((dh - 2) / 3)), lab = Math.max(10, Math.floor(dw / 10));
      let hits = 0;
      for (let k = NL - 20; k < NL; k++) hits |= lane(3, k) | 0;
      g.font = fonte(9); g.textBaseline = 'middle'; g.textAlign = 'left';
      for (let b = 0; b < 3; b++) {
        const nivel = lane(b, NL - 1);
        const fr = clamp(1 + 20 * Math.log10(Math.max(nivel, 0.001)) / 60, 0, 1);
        const y = wy + 1 + b * row;
        g.fillStyle = T.textPrimary; g.fillText('LMH'[b], wx + 2, y + row / 2);
        g.fillStyle = hits & (1 << b) ? T.warning : traco;
        g.fillRect(wx + lab + 2, y + 2, Math.floor(fr * Math.max(0, dw - lab - 8)), Math.max(1, row - 4));
      }
      rotulo = '';
    } else if (tipo === 'DYNAMICS') {
      const px = (k) => wx + 1 + k * (dw - 2) / (NL - 1);
      const yOf = (u) => wy + dh - 2 - clamp(u, 0, 1) * (dh - 4);
      const colw = Math.max(1, (dw - 2) / NL);
      g.fillStyle = comAlfa(T.warning, 0.55);
      for (let k = 0; k < NL; k++) {
        const r = clamp(lane(1, k), 0, 1);
        if (r > 0.002) g.fillRect(px(k), wy + 1, colw, r * (dh - 2));
      }
      const ty = yOf((ex(0) + 60) / 60);
      g.strokeStyle = T.textSecondary; g.lineWidth = 1; g.beginPath();
      for (let x = wx + 2; x < wx + dw - 2; x += 6) { g.moveTo(x, ty); g.lineTo(Math.min(x + 3, wx + dw - 2), ty); }
      g.stroke();
      const pts = [];
      for (let k = 0; k < NL; k++) pts.push(px(k), yOf(lane(0, k)));
      linha(pts, traco, 1.5);
      rotulo = ex(1) >= 0.5 ? 'gate' : 'comp';
    } else if (tipo === 'CELL') {
      const cells = clamp(Math.round(lane(1, NL - 1)), 1, 24);
      const colw = (dw - 2) / NL, rowh = (dh - 2) / cells;
      g.fillStyle = traco;
      for (let k = 0; k < NL; k++) {
        const row = Math.max(0, lane(0, k)) >>> 0;
        for (let i = 0; i < cells; i++)
          if ((row >>> i) & 1) g.fillRect(wx + 1 + colw * k, wy + 1 + rowh * i, Math.max(1, colw), Math.max(1, rowh - 0.5));
      }
      rotulo = 'rule ' + Math.round(ex(0));
    } else if (tipo === 'MOUSSE') {
      const colw = (dw - 2) / NL;
      g.fillStyle = comAlfa(traco, 0.35);
      const pts = [];
      for (let k = 0; k < NL; k++) {
        const top = wy + 1 + clamp(lane(0, k), 0, 1) * (dh - 3);
        g.fillRect(wx + 1 + colw * k, top, Math.max(1, colw), wy + dh - 1 - top);
        pts.push(wx + 1 + colw * k, top);
      }
      linha(pts, traco, 1.5);
      rotulo = '';
    } else if (tipo === 'KEY') {
      // o nome do tom grande, a nota que soa ao lado; embaixo o histograma
      // das 12 classes, a tônica cheia, a nota atual com contorno; a certeza
      // numa barra fina no pé
      const h = []; let mx = 1e-6;
      for (let i = 0; i < 12; i++) { h.push(ex(i)); mx = Math.max(mx, ex(i)); }
      const t = Math.round(ex(12)), agora = Math.round(ex(13));
      const x0 = wx + 2, ww = dw - 4;
      const textH = clamp(dh * 0.30, 11, 24);
      const tom = t >= 0 ? NOTAS[t] + (ex(14) >= 0.5 ? 'm' : '') : '—';
      const ty = wy + 2;
      g.font = fonte(textH, true); g.textBaseline = 'middle'; g.textAlign = 'left';
      const tomW = Math.ceil(g.measureText(tom).width);
      g.fillStyle = t >= 0 ? traco : T.textSecondary;
      g.fillText(tom, wx + 4, ty + (textH + 2) / 2);
      if (agora >= 0) {
        g.font = fonte(textH * 0.8); g.fillStyle = T.textPrimary;
        g.fillText('· ' + NOTAS[agora], wx + 4 + tomW + 6, ty + (textH + 2) / 2);
      }
      const top = ty + textH + 6, bot = wy + dh - 7, bw = ww / 12;
      if (bot - top > 4) {
        for (let k = 0; k < 12; k++) {
          const bx = x0 + bw * k + 1, bwk = Math.max(1, bw - 2), bh = (bot - top) * h[k] / mx;
          g.fillStyle = k === t ? traco : comAlfa(T.textSecondary, 0.55);
          g.fillRect(bx, bot - bh, bwk, bh);
          if (k === agora) { g.strokeStyle = T.textPrimary; g.lineWidth = 1.2; g.strokeRect(bx - 0.5, top, bwk + 1, bot - top); }
        }
      }
      g.fillStyle = comAlfa(traco, 0.8);
      g.fillRect(x0, bot + 2, ww * ex(15), 2);
      rotulo = '';
    } else if (tipo === 'COHERENCE') {
      const x0 = wx + 2, ww = dw - 4, hTop = wy + 2, hBot = wy + dh * 0.48;
      let mx = 1;
      for (let i = 0; i < 12; i++) mx = Math.max(mx, ex(i));
      const bw = ww / 12;
      g.fillStyle = comAlfa(traco, 0.75);
      for (let k = 0; k < 12; k++) {
        const bh = (hBot - hTop) * ex(k) / mx;
        g.fillRect(x0 + bw * k + 1, hBot - bh, Math.max(1, bw - 2), bh);
      }
      const lTop = wy + dh * 0.55, lBot = wy + dh - 3;
      const lx = (k) => x0 + ww * k / (NL - 1), ly = (u) => lBot - (lBot - lTop) * clamp(u, 0, 1);
      g.fillStyle = comAlfa(T.warning, 0.8);
      for (let k = 0; k < NL; k++) if (lane(2, k) > 0.5) g.fillRect(Math.floor(lx(k)), lTop, 1, lBot - lTop);
      for (let L = 0; L < 2; L++) {
        const pts = [];
        for (let k = 0; k < NL; k++) pts.push(lx(k), ly(lane(L, k)));
        linha(pts, L === 0 ? T.textPrimary : traco, L === 0 ? 1.2 : 1.6);
      }
      rotulo = '';
    } else if (tipo === 'MIXER') {
      // PALCO: o tempo desce; cada canal deixa o rastro do seu lugar
      const x0 = wx + 2, ww = dw - 4;
      const px = (p) => x0 + (clamp(p / 0.55, -1, 1) * 0.5 + 0.5) * ww;
      const top = wy + 2, bot = wy + dh - 6;
      g.fillStyle = T.line; g.fillRect(Math.floor(px(0)), top, 1, bot + 4 - top);
      const cols = [T.textPrimary, traco, T.warning, T.textSecondary];
      for (let ch = 0; ch < 4; ch++) {
        const wgt = ex(ch);
        if (wgt <= 0) continue;
        const pts = [];
        for (let k = 0; k < NL; k++) pts.push(px(lane(ch, k)), top + (bot - top) * k / (NL - 1));
        linha(pts, comAlfa(cols[ch], 0.6), 1);
        const r = 1.5 + 3.5 * clamp(wgt, 0, 1);
        g.fillStyle = cols[ch];
        g.beginPath(); g.arc(px(lane(ch, NL - 1)), bot, r, 0, 2 * Math.PI); g.fill();
      }
      if (ex(5) >= 0.5) {
        const cx = px(ex(4));
        g.strokeStyle = traco; g.lineWidth = 1.5; g.beginPath();
        g.moveTo(cx - 4, bot + 4); g.lineTo(cx + 4, bot + 4);
        g.moveTo(cx, bot + 1); g.lineTo(cx, bot + 4); g.stroke();
      }
      rotulo = '';
    } else if (tipo === 'PENDULUM') {
      const cx = wx + dw * 0.5, cy = wy + dh * 0.5, r = dh * 0.48;
      const pts = [];
      for (let k = 0; k < NL; k++) pts.push(cx + lane(2, k) * r, cy - lane(3, k) * r);
      linha(pts, comAlfa(traco, 0.5), 1);
      const x1 = cx + lane(0, NL - 1) * r, y1 = cy - lane(1, NL - 1) * r;
      const x2 = cx + lane(2, NL - 1) * r, y2 = cy - lane(3, NL - 1) * r;
      linha([cx, cy, x1, y1, x2, y2], T.textPrimary, 1.5);
      g.fillStyle = traco; g.beginPath(); g.arc(x2, y2, 2.5, 0, 2 * Math.PI); g.fill();
      rotulo = '';
    } else if (tipo === 'BOUNCE') {
      const bolas = clamp(Math.round(lane(3, NL - 1)), 1, 3);
      g.fillStyle = T.line; hline(wy + dh - 2, wx + 1, wx + dw - 1);
      const cols = [traco, T.textPrimary, T.warning];
      for (let b = 0; b < bolas; b++) {
        const pts = [];
        for (let k = 0; k < NL; k++)
          pts.push(wx + 1 + k * (dw - 2) / (NL - 1), wy + dh - 2 - clamp(lane(b, k), 0, 1) * (dh - 4));
        linha(pts, cols[b], 1.2);
      }
      rotulo = '';
    } else if (tipo === 'ARP' || tipo === 'MARKOV') {
      // as notas em degraus; com gate aberto o traço é grosso e na cor
      let lo = lane(0, 0), hi = lo;
      for (let k = 1; k < NL; k++) { lo = Math.min(lo, lane(0, k)); hi = Math.max(hi, lane(0, k)); }
      if (!isFinite(lo) || !isFinite(hi)) { lo = 0; hi = 1; }
      const mid = 0.5 * (lo + hi), span = Math.max(1, hi - lo);
      const px = (k) => wx + 1 + k * (dw - 2) / (NL - 1);
      const py = (k) => wy + dh * (0.5 - 0.84 * clamp((lane(0, k) - mid) / span, -0.5, 0.5));
      const fino = new Path2D(), grosso = new Path2D();
      for (let k = 0; k + 1 < NL; k++) {
        const p = lane(1, k) >= 0.5 ? grosso : fino;
        p.moveTo(px(k), py(k)); p.lineTo(px(k + 1), py(k));
        if (Math.abs(py(k + 1) - py(k)) > 0.5) { fino.moveTo(px(k + 1), py(k)); fino.lineTo(px(k + 1), py(k + 1)); }
      }
      g.strokeStyle = T.line; g.lineWidth = 1; g.stroke(fino);
      g.strokeStyle = traco; g.lineWidth = 2; g.stroke(grosso);
      rotulo = tipo === 'MARKOV' ? 'markov' : 'arp';
    } else if (tipo === 'PHRASE') {
      const length = Math.max(2, Math.round(ex(0)));
      const step = 1 + Math.round(lane(3, NL - 1) * (length - 1));
      const span = Math.max(1, ex(1) / 12), atual = lane(0, NL - 1);
      const pts = [];
      for (let k = 0; k < NL; k++)
        pts.push(wx + 1 + k * (dw - 2) / (NL - 1), wy + dh * (0.5 - 0.42 * clamp((lane(0, k) - atual) / span, -1, 1)));
      linha(pts, traco, 1.5);
      rotulo = `${step}/${length}`;
    } else if (tipo === 'TRIGSEQ') {
      // 4 lanes de gate rolando — piano-roll
      rotulo = 't1-t4';
      const lh = Math.max(2, Math.floor((dh - 2) / 4)), cellw = Math.max(1, Math.floor((dw - 2) / NL));
      for (let L = 0; L < 4; L++) {
        const ly = wy + 1 + L * lh;
        g.fillStyle = T.line; hline(ly + lh - 1, wx + 1, wx + dw - 2);
        g.fillStyle = traco;
        for (let k = 0; k < NL; k++)
          if (lane(L, k) >= 0.5) g.fillRect(wx + 1 + Math.floor(k * (dw - 2) / NL), ly + 1, cellw, lh - 2);
      }
    } else if (c.espectro) {
      rotulo = 'spec';
      const nb = 24, mag = espectro(v, nb);
      let mmax = 1e-6;
      for (const m of mag) mmax = Math.max(mmax, m);
      const bw = Math.max(1, Math.floor((dw - 2) / nb));
      g.fillStyle = traco;
      for (let b = 0; b < nb; b++) {
        const db = 20 * Math.log10(mag[b] / mmax + 1e-6);
        const hh = Math.floor(clamp(1 + db / 54, 0, 1) * (dh - 3));
        if (hh > 0) g.fillRect(wx + 1 + b * bw, wy + dh - 1 - hh, Math.max(1, bw - 1), hh);
      }
    } else {
      // a onda (padrão): osciloscópio da saída 0, normalizado
      const norm = 0.92 / Math.max(pico, 1e-4), meio = wy + Math.floor(dh / 2), a = Math.floor(dh / 2) - 1;
      g.fillStyle = T.line; hline(meio, wx + 1, wx + dw - 1);
      const pts = [];
      for (let k = 0; k < NS; k++) pts.push(wx + 1 + Math.floor(k * (dw - 2) / (NS - 1)), meio - v[k] * norm * a);
      linha(pts, traco, 1);
    }
  }
  if (tipo === 'QUANTIZER' && c.escala) rotulo = c.escala;
  if (rotulo) {
    g.fillStyle = T.textSecondary; g.font = fonte(9);
    g.textAlign = 'left'; g.textBaseline = 'bottom';
    g.save(); g.beginPath(); g.rect(wx + 3, wy + dh - 12, dw - 6, 11); g.clip();
    g.fillText(rotulo, wx + 3, wy + dh - 1);
    g.restore();
  }
}
