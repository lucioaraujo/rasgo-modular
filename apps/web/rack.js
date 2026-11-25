// O rack do Rasgo Modular desenhado em canvas (25/11/2026, F1 do rack no
// navegador: ver dossies/ESTUDO_outras_plataformas.md). Tradução do desenho
// do app JUCE (`RackView::relayout`, `paintChrome`, `paintWidget`,
// `paintCables` em apps/juce/RasgoModularApp.cpp) com as mesmas medidas,
// cores e regras de rótulo; o painel de cada módulo vem do próprio motor
// (`Signal::panel()`, `rw_descrever`), não de uma cópia.
//
// Custo de desenho (lição do Pong, 2026): a camada fixa de cada módulo é
// rasterizada UMA vez num canvas próprio e depois só copiada; densidade de
// pixel limitada a 1,5; nada de sombra borrada; o canvas tem o tamanho da
// janela, não do rack, e só os módulos à vista são desenhados.

const T = {
  bg: '#131a1a', surface: '#262b36', recessed: '#0e1015', line: '#485060',
  textPrimary: '#f0f3f8', textSecondary: '#bec8d8', accent: '#ff9d4c',
  warning: '#ff6b5b', prata: '#d8dde6',
};
const CABO_AUDIO = ['#d97b4a', '#c96f6f', '#d9a24a', '#b5734f'];
const CABO_CTRL = ['#5cb0d4', '#6f96c9', '#4aa9b5', '#7f8fd9'];
const MATIZ = { SOURCE: 90, TRANSFORM: 285, MODULATE: 325, TIME: 150, DECISION: 52,
                ROUTE: 195, SPACE: 245, OUT: 0 };
const K = { Knob: 0, Slider: 1, Toggle: 2, Jack: 3, Display: 4, Label: 5 };
const MM_HP = 5.08, MM_3U = 128.5;
const S_MIN = 1.6, S_MAX = 2.6, FILEIRAS = 3, PAD = 14, TOPO = 2, VAO_MM = 3;
const DPR_MAX = 1.5;
const WM = { x: 6.63, y: 12.62, w: 258.74, h: 30.75 };

function corFamilia(f, alfa = 1) {
  if (!(f in MATIZ)) return T.line;
  const sat = f === 'OUT' ? 12 : 34;
  return `hsla(${MATIZ[f]}, ${sat}%, 56%, ${alfa})`;
}
const fonte = (px, negrito) => `${negrito ? 'bold ' : ''}${px * 0.86}px system-ui, sans-serif`;
const clamp = (v, a, b) => Math.min(b, Math.max(a, v));
const diamKnob = (w) => (w.s > 1 ? w.s : 8);
const sliderTempo = (tipo, b) => tipo === 'ENVELOPE' && (b === 'attack' || b === 'decay' || b === 'release');
const celulaMatriz = (b) => b.length === 3 && b[0] === 'g' && b[1] >= '1' && b[1] <= '4' && b[2] >= '1' && b[2] <= '4';

// texto centrado na caixa, comprimido na horizontal até 70% se não couber
// (o `drawFittedText` do JUCE)
function textoAjustado(g, txt, x, y, w, h, alinhar = 'centro') {
  const tw = g.measureText(txt).width;
  const esc = tw > w ? Math.max(0.7, w / tw) : 1;
  g.save();
  g.textBaseline = 'middle';
  const cy = y + h / 2;
  if (alinhar === 'esquerda') {
    g.translate(x, cy); g.scale(esc, 1); g.textAlign = 'left'; g.fillText(txt, 0, 0);
  } else {
    g.translate(x + w / 2, cy); g.scale(esc, 1); g.textAlign = 'center'; g.fillText(txt, 0, 0);
  }
  g.restore();
}

export class Rack {
  constructor(canvas, rolagem) {
    this.cv = canvas;
    this.g = canvas.getContext('2d');
    this.rolagem = rolagem;          // elemento que rola (o espaçador dá a altura)
    this.mods = [];
    this.vistos = [];
    this.valores = null;
    this.cabos = null;
    this.marca = null;
    this.escala = 2;
    this.pedido = false;
    this.soSaida = false;   // vista SAÍDA: só os módulos que chegam ao som
    addEventListener('resize', () => this.layout());
    rolagem.addEventListener('scroll', () => this.redesenhar(), { passive: true });
  }

  mmpx(mm) { return Math.round(mm * this.escala); }

  vistaSaida(v) { this.soSaida = v; this.layout(); }

  carregar(texto, wordmark) {
    const r = JSON.parse(texto);
    this.saida = r.saida;
    this.mods = r.modulos.map((m) => {
      const w = m.w.map((x) => ({ ...x }));
      if (m.inst > 0 && w.length && w[0].k === K.Label) w[0].l += ' ' + m.inst;
      return { ...m, w, cromo: null };
    });
    if (wordmark) this.marca = new Path2D(wordmark);
    this.valores = null;
    this.cabos = null;
    this.layout();
  }

  quadro(valores, cabos) {
    this.valores = valores;
    this.cabos = cabos;
    this.redesenhar();
  }

  // `relayout` do app: a escala faz caber três fileiras na altura da janela
  layout() {
    const dpr = Math.min(devicePixelRatio || 1, DPR_MAX);
    this.dpr = dpr;
    const vw = this.rolagem.clientWidth, vh = this.rolagem.clientHeight;
    const budget = (Math.max(120, vh) - TOPO) / FILEIRAS;
    this.escala = clamp((budget - PAD - 0.5) / MM_3U, S_MIN, S_MAX);
    this.modH = this.mmpx(MM_3U);
    let x = PAD, y = TOPO;
    this.vistos = this.soSaida ? this.mods.filter((m) => m.chega) : this.mods;
    for (const m of this.vistos) {
      const w = this.mmpx(m.hp * MM_HP);
      if (x + w > vw - PAD && x > PAD) { x = PAD; y += this.modH + PAD; }
      m.bx = x; m.by = y; m.bw = w; m.bh = this.modH;
      m.cromo = null;
      x += w + this.mmpx(VAO_MM);
    }
    this.alturaConteudo = y + this.modH + PAD;
    this.layoutN = (this.layoutN || 0) + 1;
    this.rolagem.querySelector('.espaco').style.height = this.alturaConteudo + 'px';
    this.cv.width = Math.round(vw * dpr);
    this.cv.height = Math.round(vh * dpr);
    this.cv.style.width = vw + 'px';
    this.cv.style.height = vh + 'px';
    // posição de tela de cada jack (`rebuildJacks`)
    this.jacks = new Map();
    for (const m of this.vistos)
      for (const w of m.w)
        if (w.k === K.Jack && w.p !== undefined)
          this.jacks.set(`${m.id}:${w.p}:${w.out}`, { x: m.bx + this.mmpx(w.x), y: m.by + this.mmpx(w.y), pk: w.pk });
    this.redesenhar();
  }

  redesenhar() {
    if (this.pedido) return;
    this.pedido = true;
    requestAnimationFrame(() => { this.pedido = false; this.desenhar(); });
  }

  // Um quadro: a camada fixa de cada módulo à vista é copiada; o que se
  // mexe (ponteiros, sliders, botões) é juntado por cor em poucos traços —
  // centenas de controles viram meia dúzia de chamadas de desenho.
  desenhar() {
    const g = this.g, dpr = this.dpr;
    const vw = this.cv.width / dpr, vh = this.cv.height / dpr;
    const topo = this.rolagem.scrollTop;
    g.setTransform(dpr, 0, 0, dpr, 0, 0);
    g.fillStyle = T.bg;
    g.fillRect(0, 0, vw, vh);
    g.translate(0, -topo);
    const ponteiros = new Path2D(), ligados = new Path2D(), desligados = new Path2D();
    const bordas = new Path2D(), brilhos = new Path2D(), pegas = [];
    const cheios = new Map();   // cor da família → retângulos dos sliders
    for (const m of this.vistos) {
      if (m.by + m.bh < topo || m.by > topo + vh) continue;
      if (!m.cromo) m.cromo = this.cromo(m);
      g.drawImage(m.cromo, m.bx, m.by, m.bw, m.bh);
      for (const w of m.w) {
        if (w.v === undefined) continue;
        if (m.tipo === 'MATRIX' && w.k === K.Knob && celulaMatriz(w.b)) continue;
        const wx = m.bx + this.mmpx(w.x), wy = m.by + this.mmpx(w.y);
        if (w.k === K.Knob) {
          // ponteiro: -135° a +135°, como no app
          const r = this.mmpx(diamKnob(w) * 0.5), cx = wx + r, cy = wy + r;
          const a = (-135 + 270 * this.norma(m, w)) * Math.PI / 180;
          ponteiros.moveTo(cx, cy);
          ponteiros.lineTo(cx + Math.sin(a) * r * 0.8, cy - Math.cos(a) * r * 0.8);
        } else if (w.k === K.Slider) {
          const bw = this.mmpx(6), bh = this.mmpx(30), fundo = wy + bh;
          const fill = Math.round(bh * this.norma(m, w));
          if (fill > 1) {
            if (!cheios.has(m.familia)) cheios.set(m.familia, new Path2D());
            cheios.get(m.familia).rect(wx + 1, fundo - fill, bw - 2, fill - 1);
          }
          pegas.push(wx - 1, Math.min(fundo - 4, fundo - fill - 2), bw + 2);
        } else if (w.k === K.Toggle) {
          const sz = this.mmpx(4);
          (this.valor(w) >= 0.5 ? ligados : desligados).rect(wx, wy, sz, sz);
          bordas.rect(wx + 0.5, wy + 0.5, sz - 1, sz - 1);
          brilhos.moveTo(wx + 1, wy + 1.5); brilhos.lineTo(wx + sz - 1, wy + 1.5);
          brilhos.moveTo(wx + 1.5, wy + 1); brilhos.lineTo(wx + 1.5, wy + sz - 1);
        }
      }
    }
    for (const [fam, p] of cheios) { g.fillStyle = corFamilia(fam); g.fill(p); }
    // a pega prateada do slider: um degradê pronto, só copiado
    const pega = this.pega();
    for (let i = 0; i < pegas.length; i += 3) g.drawImage(pega, pegas[i], pegas[i + 1], pegas[i + 2], 4);
    g.fillStyle = T.prata; g.fill(ligados);
    g.fillStyle = T.recessed; g.fill(desligados);
    g.strokeStyle = T.line; g.lineWidth = 1; g.stroke(bordas);
    g.strokeStyle = 'rgba(216, 221, 230, 0.56)'; g.stroke(brilhos);
    g.strokeStyle = T.prata; g.lineWidth = 2; g.lineCap = 'butt'; g.stroke(ponteiros);
    // os cabos: uma camada própria, refeita só quando um cabo muda (ganho,
    // condução, ruptura) ou a vista rola — redesenhar 63 cabos atravessando
    // a tela custava ~6 ms por quadro
    const chave = `${topo}|${vw}x${vh}|${this.layoutN}|${this.chaveFixos()}`;
    if (chave !== this.chaveCabos) {
      this.chaveCabos = chave;
      const cc = this.camadaCabos ||= document.createElement('canvas');
      if (cc.width !== this.cv.width || cc.height !== this.cv.height) { cc.width = this.cv.width; cc.height = this.cv.height; }
      const gc = cc.getContext('2d');
      gc.setTransform(1, 0, 0, 1, 0, 0);
      gc.clearRect(0, 0, cc.width, cc.height);
      gc.setTransform(dpr, 0, 0, dpr, 0, -topo * dpr);
      this.desenharCabos(gc, topo, vh, true);
    }
    // os que piscam, por cima, a cada quadro
    this.desenharCabos(g, topo, vh, false);
    g.setTransform(1, 0, 0, 1, 0, 0);
    g.drawImage(this.camadaCabos, 0, 0);
  }

  pega() {
    if (this._pega) return this._pega;
    const c = document.createElement('canvas');
    c.width = 1; c.height = 16;
    const g = c.getContext('2d');
    const gr = g.createLinearGradient(0, 0, 0, 16);
    gr.addColorStop(0, '#e2e6ee'); gr.addColorStop(1, '#8a93a4');
    g.fillStyle = gr; g.fillRect(0, 0, 1, 16);
    return (this._pega = c);
  }

  // ---- camada fixa (`paintChrome`) ------------------------------------
  cromo(m) {
    const dpr = this.dpr;
    const c = document.createElement('canvas');
    c.width = Math.max(1, Math.round(m.bw * dpr));
    c.height = Math.max(1, Math.round(m.bh * dpr));
    const g = c.getContext('2d');
    g.setTransform(dpr, 0, 0, dpr, 0, 0);
    // coordenadas absolutas, como no app (`setOrigin(-bounds)`)
    g.translate(-m.bx, -m.by);
    g.fillStyle = T.surface;
    g.fillRect(m.bx, m.by, m.bw, m.bh);
    if (m.familia) {
      const gr = g.createLinearGradient(m.bx + 1, 0, m.bx + m.bw - 1, 0);
      gr.addColorStop(0, corFamilia(m.familia));
      gr.addColorStop(1, corFamilia(m.familia, 0));
      g.fillStyle = gr;
      g.fillRect(m.bx + 1, m.by + 1, m.bw - 2, 3);
    }
    g.strokeStyle = T.line;
    g.lineWidth = 1;
    g.beginPath();
    g.moveTo(m.bx + 2, m.by + 3.5); g.lineTo(m.bx + m.bw - 2, m.by + 3.5);
    g.moveTo(m.bx + 2, m.by + m.bh - 2.5); g.lineTo(m.bx + m.bw - 2, m.by + m.bh - 2.5);
    g.stroke();
    {  // [x] de remover
      const xx = m.bx + m.bw - 12, xy = m.by + 5;
      g.strokeStyle = T.textSecondary;
      g.beginPath();
      g.moveTo(xx, xy); g.lineTo(xx + 6, xy + 6);
      g.moveTo(xx + 6, xy); g.lineTo(xx, xy + 6);
      g.stroke();
    }
    g.save();
    g.beginPath();
    g.rect(m.bx + 1, m.by + 1, m.bw - 2, m.bh - 2);
    g.clip();
    for (const w of m.w) {
      if (m.tipo === 'MATRIX' && w.k === K.Knob && celulaMatriz(w.b)) continue;
      this.widgetFixo(g, m, w);
    }
    this.wordmark(g, m);
    // o que no app é passada dinâmica mas aqui não muda (ainda): o anel do
    // jack sem cabo sendo puxado e o fundo dos visores (F3)
    for (const w of m.w) {
      const wx = m.bx + this.mmpx(w.x), wy = m.by + this.mmpx(w.y);
      if (w.k === K.Jack) {
        g.strokeStyle = T.line; g.lineWidth = 1.5;
        g.beginPath(); g.arc(wx, wy, this.mmpx(2.4) - 0.75, 0, 2 * Math.PI); g.stroke();
      }
    }
    if (m.tipo === 'MATRIX') {
      g.fillStyle = T.textSecondary;
      g.font = fonte(9);
      textoAjustado(g, 'IN ↓   OUT →', m.bx + this.mmpx(50), m.by + this.mmpx(16), this.mmpx(40), this.mmpx(5), 'esquerda');
    }
    g.restore();
    g.strokeStyle = T.line; g.lineWidth = 1;
    g.strokeRect(m.bx + 0.5, m.by + 0.5, m.bw - 1, m.bh - 1);
    return c;
  }

  // `paintWordmark`: 22 mm, na base; no centro, ou de lado se esbarrar
  wordmark(g, m) {
    if (!this.marca) return;
    const lw = this.mmpx(22);
    if (lw > m.bw - 6) return;
    const lh = lw * WM.h / WM.w;
    const topo = m.by + m.bh - 5 - lh;
    const bate = (xa, xb) => {
      for (const w of m.w) {
        let cx, meia, fundo;
        if (w.k === K.Jack) { cx = this.mmpx(w.x); meia = this.mmpx(3); fundo = this.mmpx(w.y + 2.6); }
        else if (w.k === K.Knob) { const d = diamKnob(w); cx = this.mmpx(w.x + d * 0.5); meia = this.mmpx(d * 0.6); fundo = this.mmpx(w.y + d); }
        else if (w.k === K.Toggle) { cx = this.mmpx(w.x + 2); meia = this.mmpx(3); fundo = this.mmpx(w.y + 8); }
        else continue;
        cx += m.bx; fundo += m.by;
        if (cx + meia > xa && cx - meia < xb && fundo > topo - 2) return true;
      }
      return false;
    };
    let x = m.bx + m.bw / 2 - lw * 0.5;
    if (bate(x, x + lw)) {
      const xe = m.bx + 4, xd = m.bx + m.bw - 4 - lw;
      if (!bate(xe, xe + lw)) x = xe;
      else if (!bate(xd, xd + lw)) x = xd;
    }
    g.save();
    g.translate(x, topo);
    g.scale(lw / WM.w, lw / WM.w);
    g.translate(-WM.x, -WM.y);
    g.fillStyle = 'rgba(216, 221, 230, 0.85)';
    g.fill(this.marca);
    g.restore();
  }

  // rótulo centrado, mas sempre dentro do módulo (`rotulo`)
  rotulo(g, m, txt, x, y, w, h) {
    const tw = Math.min(w, Math.ceil(g.measureText(txt).width) + 2);
    const lo = m.bx + 2, hi = m.bx + m.bw - 2;
    const rw = Math.min(tw, hi - lo);
    const rx = clamp(x + (w - tw) / 2, lo, hi - rw);
    textoAjustado(g, txt, rx, y, rw, h);
  }

  rotuloCabeADireita(g, m, w) {
    const tw = g.measureText(w.l).width;
    let limite = m.bx + m.bw - 3;
    for (const o of m.w) {
      if (o === w || o.k === K.Label) continue;
      let ox = o.x, oy0 = o.y, oy1 = o.y + 4;
      if (o.k === K.Jack) { ox = o.x - 2.4; oy0 = o.y - 7; oy1 = o.y + 2.4; }
      else if (o.k === K.Knob) { ox = o.x - 2; oy0 = o.y - 4; oy1 = o.y + diamKnob(o); }
      else if (o.k === K.Slider) { ox = o.x - 2; oy1 = o.y + 33.5; }
      else if (o.k === K.Display) { oy1 = o.y + o.h; }
      if (ox <= w.x || oy1 < w.y || oy0 > w.y + 4) continue;
      limite = Math.min(limite, m.bx + this.mmpx(ox) - 2);
    }
    return m.bx + this.mmpx(w.x) + this.mmpx(4) + 2 + tw <= limite;
  }

  widgetFixo(g, m, w) {
    const wx = m.bx + this.mmpx(w.x), wy = m.by + this.mmpx(w.y);
    g.font = fonte(9);
    g.fillStyle = T.textSecondary;
    switch (w.k) {
      case K.Knob: {
        const r = this.mmpx(diamKnob(w) * 0.5);
        const cx = wx + r, cy = wy + r;
        const gr = g.createLinearGradient(0, wy, 0, wy + 2 * r);
        gr.addColorStop(0, '#2c303a');   // recessed.brighter(0.35)
        gr.addColorStop(1, T.recessed);
        g.fillStyle = gr;
        g.beginPath(); g.arc(cx, cy, r, 0, 2 * Math.PI); g.fill();
        g.strokeStyle = T.line; g.lineWidth = 1;
        g.beginPath(); g.arc(cx, cy, r - 0.5, 0, 2 * Math.PI); g.stroke();
        const rf = g.createLinearGradient(wx, wy, wx + 2 * r * 0.8, wy + 2 * r * 0.8);
        rf.addColorStop(0, 'rgba(216, 221, 230, 0.82)');
        rf.addColorStop(1, 'rgba(216, 221, 230, 0)');
        g.strokeStyle = rf; g.lineWidth = 1.4;
        g.beginPath(); g.arc(cx, cy, r - 1.5, 0, 2 * Math.PI); g.stroke();
        g.fillStyle = T.textSecondary;
        this.rotulo(g, m, w.l, wx - this.mmpx(2), wy - this.mmpx(4), 2 * r + this.mmpx(4), this.mmpx(3.5));
        break;
      }
      case K.Slider: {
        const bw = this.mmpx(6), bh = this.mmpx(30);
        g.fillStyle = T.recessed; g.fillRect(wx, wy, bw, bh);
        g.strokeStyle = T.line; g.lineWidth = 1; g.strokeRect(wx + 0.5, wy + 0.5, bw - 1, bh - 1);
        g.fillStyle = T.textSecondary;
        this.rotulo(g, m, w.l, wx - this.mmpx(2), wy + bh, bw + this.mmpx(4), this.mmpx(3.5));
        break;
      }
      case K.Toggle: {
        const s = this.mmpx(4);
        if (w.s > 1) {
          const width = this.mmpx(w.s);
          textoAjustado(g, w.l, wx + s / 2 - width / 2, wy + s + this.mmpx(1), width, this.mmpx(3.5));
        } else {
          let aDireita = this.rotuloCabeADireita(g, m, w);
          for (const o of m.w)
            if (aDireita && o !== w && o.k === K.Toggle && o.s <= 1 && Math.abs(o.y - w.y) < 0.5
                && !this.rotuloCabeADireita(g, m, o)) aDireita = false;
          if (aDireita) textoAjustado(g, w.l, wx + s + 2, wy, 1e6, s, 'esquerda');
          else {
            const width = this.mmpx(9);
            this.rotulo(g, m, w.l, wx + s / 2 - width / 2, wy + s + this.mmpx(1), width, this.mmpx(3.5));
          }
        }
        break;
      }
      case K.Jack: {
        const r = this.mmpx(2.4);
        g.fillStyle = T.recessed;
        g.beginPath(); g.arc(wx, wy, r, 0, 2 * Math.PI); g.fill();
        g.fillStyle = T.textSecondary;
        this.rotulo(g, m, w.l, wx - this.mmpx(5), wy - this.mmpx(6.5), this.mmpx(10), this.mmpx(3.5));
        break;
      }
      case K.Display: {
        let dw = this.mmpx(w.s > 1 ? w.s : 16);
        dw = Math.min(dw, m.bw - this.mmpx(w.x) - this.mmpx(2));
        const dh = this.mmpx(w.h);
        g.fillStyle = T.recessed; g.fillRect(wx, wy, dw, dh);
        g.strokeStyle = T.line; g.lineWidth = 1; g.strokeRect(wx + 0.5, wy + 0.5, dw - 1, dh - 1);
        break;
      }
      case K.Label: {
        const titulo = w === m.w[0];
        g.fillStyle = titulo ? T.textPrimary : T.textSecondary;
        g.font = fonte(titulo ? 12 : 10, titulo);
        const lw = titulo ? Math.max(this.mmpx(10), m.bx + m.bw - this.mmpx(5.5) - wx) : this.mmpx(24);
        textoAjustado(g, w.l, wx, wy, lw, this.mmpx(titulo ? 4.2 : 3.5), 'esquerda');
        break;
      }
    }
  }

  valor(w) {
    if (w.v === undefined || !this.valores) return w.def ?? 0;
    return this.valores[w.v];
  }

  norma(m, w) {
    if (w.v === undefined || !(w.max > w.min)) return 0;
    const v = this.valor(w);
    if (w.k === K.Slider && sliderTempo(m.tipo, w.b)) {
      if (!(w.min > 0)) return 0;
      return clamp(Math.log(clamp(v, w.min, w.max) / w.min) / Math.log(w.max / w.min), 0, 1);
    }
    return clamp((v - w.min) / (w.max - w.min), 0, 1);
  }

  // ---- cabos (`paintCables` + `strokeCable`) --------------------------
  // `fixos`: true = só os cabos de condução 1 (a camada guardada), false =
  // só os que piscam (redesenhados a cada quadro)
  desenharCabos(g, topo, vh, fixos) {
    const c = this.cabos;
    if (!c) return;
    for (let i = 0; i + 8 < c.length; i += 9) {
      const [sn, sp, tn, tp, rompido, conduz, ganho, , condut] = c.subarray(i, i + 9);
      if ((condut >= 1) !== fixos) continue;
      const s = this.jacks.get(`${sn}:${sp}:1`), t = this.jacks.get(`${tn}:${tp}:0`);
      if (!s || !t) continue;   // uma das pontas é o sink (não exibido)
      const sag = 18 + Math.abs(t.x - s.x) / 6, baixo = Math.max(s.y, t.y) + sag;
      if (Math.min(s.y, t.y) > topo + vh || baixo < topo) continue;
      const h = sn * 7 + sp * 3 + tn * 5 + tp;
      const cor = rompido ? T.warning : (s.pk === 1 ? CABO_CTRL[h & 3] : CABO_AUDIO[h & 3]);
      g.globalAlpha = !rompido && !conduz ? 0.22 : 1;
      this.tracarCabo(g, s.x, s.y, t.x, t.y, cor, rompido, 1 + clamp(ganho, 0, 2));
      g.globalAlpha = 1;
    }
  }

  // a chave da camada guardada: tudo dos cabos de condução 1, menos o
  // sorteio de condução (que neles não muda)
  chaveFixos() {
    const c = this.cabos;
    if (!c) return '';
    let k = '';
    for (let i = 0; i + 8 < c.length; i += 9)
      if (c[i + 8] >= 1) k += `${c[i]},${c[i + 1]},${c[i + 2]},${c[i + 3]},${c[i + 4]},${c[i + 6]};`;
    return k;
  }

  tracarCabo(g, x0, y0, x1, y1, cor, tracejado, largura = 2) {
    const sag = 18 + Math.abs(x1 - x0) / 6;
    const cx = (x0 + x1) / 2, cy = Math.max(y0, y1) + sag;
    g.beginPath();
    for (let k = 0; k < 15; k++) {
      const t = k / 14, u = 1 - t;
      const x = u * u * x0 + 2 * u * t * cx + t * t * x1;
      const y = u * u * y0 + 2 * u * t * cy + t * t * y1;
      if (k === 0) g.moveTo(x, y); else g.lineTo(x, y);
    }
    g.strokeStyle = cor;
    g.lineJoin = 'round';
    if (tracejado) { g.setLineDash([5, 4]); g.lineWidth = largura * 0.5; }
    else g.lineWidth = largura;
    g.stroke();
    g.setLineDash([]);
  }
}
