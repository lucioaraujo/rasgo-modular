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

import { VISOR, desenharVisor } from './visores.js';

const T = {
  bg: '#131a1a', surface: '#262b36', recessed: '#0e1015', line: '#485060',
  textPrimary: '#f0f3f8', textSecondary: '#bec8d8', accent: '#d8dde6',   // prata: o laranja saiu de vez (autor, 27/11/2026)
 
  warning: '#ff6b5b', prata: '#d8dde6',
  placa: '#3a4252',   // `ui::kOutputPlate`
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
// as luzes das saídas: as mesmas constantes de `src/ui/OutputLights.hpp`
const LUZ = { dx: 3.4, dy: -2.6, r: 0.8, pos: [143, 227, 154], neg: [255, 107, 91], decai: 0.82 };

export function corFamilia(f, alfa = 1) {
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
    this.zoom = 1;          // ZOOM − + do cabeçalho (`nudgeZoom`: 0,55–1,40)
    this.pedido = false;
    this.soSaida = false;   // vista SAÍDA: só os módulos que chegam ao som
    this.pendentes = new Set();   // à vista na SAÍDA por exceção (cortados agora)
    this.enviar = () => {};       // mensagens ao motor (o worklet)
    this.gesto = null;            // o que o mouse está fazendo
    this.insp = null;             // inspetor: { d, pd } = a ponta de destino do cabo
    this.escolhendo = false;      // escolhendo o companion da relação
    this.sobCabo = -1;            // índice do cabo sob o mouse (destaque)
    this.aviso = () => {};        // texto de aviso para a página
    this.tx = (k) => k;           // os textos do app na língua escolhida (a página põe)
    this.mouse = { x: 0, y: 0 };
    this.visores = new Map();     // id → o visor agora (Float32Array, `rw_visor`)
    this.escalas = {};            // QUANTIZER com BANCO: id → nome da escala
    this.espectro = new Set();    // SCOPE mostrando o espectro (clique no visor)
    this.clipAte = new Map();     // MASTER: até quando o aviso de limite fica aceso
    this.idsVisiveis = '';
    this.paletaTipo = '';         // tipo sob o mouse na paleta: os módulos dele acendem
    this.fantasma = null;         // { tipo, x, y }: o módulo vindo da paleta
    this.sobrePaleta = () => false;   // a página diz se o ponto está sobre a paleta
    this.abrirBanco = () => {};       // QUANTIZER: clique no visor abre o banco de escalas
    this.ultimaOrdem = 0;
    this.opacidade = 0.5;         // dos cabos: 50%, como o VCV (autor, 25/11; `ui::kCableOpacity` no app)
    this.silenciado = false;      // ESPERA: os cabos esmaecem (32%), como no app
    // a área do rack muda de altura sem a janela mudar (o cabeçalho quebra em
    // duas linhas quando os textos da língua entram, o REC aparece…): medir
    // de novo, senão a base da última fileira fica cortada (relato do autor,
    // 26/11)
    addEventListener('resize', () => this.layout());
    if (typeof ResizeObserver !== 'undefined') {
      let medida = '';
      new ResizeObserver(() => {
        const m = `${rolagem.clientWidth}x${rolagem.clientHeight}`;
        if (m !== medida) { medida = m; requestAnimationFrame(() => this.layout()); }   // no quadro seguinte: sem laço
      }).observe(rolagem);
    }
    rolagem.addEventListener('scroll', () => this.redesenhar(), { passive: true });
    canvas.addEventListener('pointerdown', (e) => this.baixa(e));
    canvas.addEventListener('pointermove', (e) => this.move(e));
    canvas.addEventListener('pointerup', (e) => this.sobe(e));
    canvas.addEventListener('pointercancel', () => this.cancela());
    canvas.addEventListener('contextmenu', (e) => e.preventDefault());
    // o botão do meio é do rack (arrastar), não da rolagem automática do navegador
    canvas.addEventListener('mousedown', (e) => { if (e.button === 1) e.preventDefault(); });
    canvas.addEventListener('wheel', (e) => this.roda(e), { passive: false });
  }

  mmpx(mm) { return Math.round(mm * this.escala); }

  // `nudgeZoom`: ±0,10, entre 0,55 e 1,40; 0 volta a 1
  mudarZoom(dir) {
    this.zoom = dir === 0 ? 1 : clamp(this.zoom + 0.1 * dir, 0.55, 1.4);
    this.layout();
  }

  vistaSaida(v) { this.soSaida = v; this.pendentes.clear(); this.layout(); }

  // `mesmoPatch`: a descrição veio de um cabo ligado/cortado (não de uma
  // seed nova) — na vista SAÍDA, quem deixou de chegar ao som fica à vista,
  // tracejado, até voltar a chegar (`keepVisibleAfterCut` do app)
  carregar(texto, wordmark, mesmoPatch = false) {
    this.textoAtual = texto;   // para refazer os painéis ao ligar/desligar o COMPACTO
    const r = JSON.parse(texto);
    // 'manter': o mesmo patch redesenhado (o COMPACTO), sem mexer em nada
    const manter = mesmoPatch === 'manter';
    if (manter) { /* pendentes, valores e cabos ficam */ }
    else if (mesmoPatch) {
      for (const m of this.vistos) this.pendentes.add(m.id);
    } else this.pendentes.clear();
    this.saida = r.saida;
    this.mods = r.modulos.map((m) => {
      const w = m.w.map((x) => ({ ...x }));
      if (m.inst > 0 && w.length && w[0].k === K.Label) w[0].l += ' ' + m.inst;
      return { ...m, w, cromo: null };
    });
    if (wordmark) this.marca = new Path2D(wordmark);
    for (const m of this.mods) if (m.chega) this.pendentes.delete(m.id);
    if (!mesmoPatch) { this.valores = null; this.cabos = null; }
    if (manter) this.chaveCabos = null;
    this.layout();
  }

  quadro(valores, cabos, ids = [], visores = null, escalas = {}, luzes = null) {
    // as luzes das saídas (`ui::lightFollow`): a leitura nova ou a de antes caindo
    if (luzes) {
      if (!this.luzes || this.luzes.length !== luzes.length) this.luzes = new Float32Array(luzes.length);
      for (let k = 0; k < luzes.length; k++) {
        const caindo = this.luzes[k] * LUZ.decai;
        this.luzes[k] = Math.abs(luzes[k]) >= Math.abs(caindo) ? luzes[k] : caindo;
      }
    }
    this.valores = valores;
    this.cabos = cabos;
    this.visores.clear();
    if (visores) ids.forEach((id, k) => this.visores.set(id, visores.subarray(k * VISOR, (k + 1) * VISOR)));
    this.escalas = escalas;
    this.redesenhar();
  }

  // os visores que estão na tela: só deles o motor manda a telemetria
  visiveis(topo, vh) {
    const ids = [];
    for (const m of this.vistos)
      if (m.by + m.bh >= topo && m.by <= topo + vh && m.w.some((w) => w.k === K.Display)) ids.push(m.id);
    const chave = ids.join(',');
    if (chave !== this.idsVisiveis) { this.idsVisiveis = chave; this.enviar({ tipo: 'visiveis', ids }); }
  }

  // `relayout` do app: a escala faz caber três fileiras na altura da janela
  layout() {
    const dpr = Math.min(devicePixelRatio || 1, DPR_MAX);
    this.dpr = dpr;
    const vw = this.rolagem.clientWidth, vh = this.rolagem.clientHeight;
    const budget = (Math.max(120, vh) - TOPO) / FILEIRAS;
    this.escala = clamp((budget - PAD - 0.5) / MM_3U, S_MIN, S_MAX) * this.zoom;
    this.modH = this.mmpx(MM_3U);
    let x = PAD, y = TOPO;
    this.vistos = this.soSaida ? this.mods.filter((m) => m.chega || this.pendentes.has(m.id)) : this.mods;
    for (const m of this.vistos) {
      const w = this.mmpx(m.hp * MM_HP);
      if (x + w > vw - PAD && x > PAD) { x = PAD; y += this.modH + PAD; }
      // a camada fixa não depende da posição (só do tamanho e da escala):
      // reordenar não a refaz
      const chave = `${w}x${this.modH}@${this.escala}/${dpr}`;
      if (m.chaveCromo !== chave) { m.cromo = null; m.chaveCromo = chave; }
      m.bx = x; m.by = y; m.bw = w; m.bh = this.modH;
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
          this.jacks.set(`${m.id}:${w.p}:${w.out}`, { x: m.bx + this.mmpx(w.x), y: m.by + this.mmpx(w.y), pk: w.pk, no: m.id, p: w.p, out: w.out });
    this.porId = new Map(this.mods.map((m) => [m.id, m]));
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
    const ponteiros = new Path2D(), escuros = new Path2D(), ligados = new Path2D(), desligados = new Path2D();
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
          const pp = w.pr ? escuros : ponteiros;   // no principal (claro), ponteiro escuro
          pp.moveTo(cx, cy);
          pp.lineTo(cx + Math.sin(a) * r * 0.8, cy - Math.cos(a) * r * 0.8);
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
    g.strokeStyle = T.bg; g.stroke(escuros);
    this.visiveis(topo, vh);
    const agora = performance.now();
    for (const m of this.vistos) {
      if (m.by + m.bh < topo || m.by > topo + vh) continue;
      if (m.tipo === 'MATRIX') this.desenharMatriz(g, m);
      const v = this.visores.get(m.id);
      for (const w of m.w) {
        if (w.k !== K.Display) continue;
        let dw = this.mmpx(w.s > 1 ? w.s : 16);
        dw = Math.min(dw, m.bw - this.mmpx(w.x) - this.mmpx(2));
        const wx = m.bx + this.mmpx(w.x), wy = m.by + this.mmpx(w.y);
        if (m.tipo === 'MASTER' && v && v[VISOR - 16] > 0.05) this.clipAte.set(m.id, agora + 2000);
        g.save();
        g.beginPath();
        if (w.rd) g.arc(wx + dw / 2, wy + this.mmpx(w.h) / 2, Math.min(dw, this.mmpx(w.h)) / 2 - 1, 0, 2 * Math.PI);
        else g.rect(wx + 1, wy + 1, dw - 2, this.mmpx(w.h) - 2);
        g.clip();
        desenharVisor(g, { T, wx, wy, dw, dh: this.mmpx(w.h), tipo: m.tipo, v, rotulo: w.l,
          traco: !m.familia || m.familia === 'OUT' ? T.prata : corFamilia(m.familia),
          espectro: this.espectro.has(m.id), clip: (this.clipAte.get(m.id) || 0) > agora,
          escala: this.escalas[m.id] });
        g.restore();
      }
    }
    // as luzes acesas: verde para o positivo, vermelho para o negativo
    if (this.luzes) {
      const r = this.mmpx(LUZ.r);
      for (const m of this.vistos) {
        if (m.by + m.bh < topo || m.by > topo + vh) continue;
        for (const w of m.w) {
          if (w.li === undefined) continue;
          const v = this.luzes[w.li] || 0, a = Math.abs(v);
          if (a < 0.01) continue;
          const c = v > 0 ? LUZ.pos : LUZ.neg, b = Math.sqrt(Math.min(1, a));
          const x = m.bx + this.mmpx(w.x + LUZ.dx), y = m.by + this.mmpx(w.y + LUZ.dy);
          g.fillStyle = `rgba(${c[0]}, ${c[1]}, ${c[2]}, ${b})`;
          g.beginPath(); g.arc(x, y, r, 0, 2 * Math.PI); g.fill();
          if (b > 0.5) {   // o halo de um LED aceso
            g.fillStyle = `rgba(${c[0]}, ${c[1]}, ${c[2]}, ${0.18 * b})`;
            g.beginPath(); g.arc(x, y, r * 2.2, 0, 2 * Math.PI); g.fill();
          }
        }
      }
    }
    // borda acesa: o módulo sendo arrastado e os do tipo sob o mouse na paleta
    const arrastado = this.gesto?.tipo === 'modulo' && this.gesto.movido ? this.gesto.id : -1;
    g.strokeStyle = T.accent; g.lineWidth = 1;
    for (const m of this.vistos) {
      if (m.id !== arrastado && (!this.paletaTipo || m.tipo !== this.paletaTipo)) continue;
      if (m.by + m.bh < topo || m.by > topo + vh) continue;
      g.strokeRect(m.bx + 0.5, m.by + 0.5, m.bw - 1, m.bh - 1);
      g.strokeRect(m.bx - 0.5, m.by - 0.5, m.bw + 1, m.bh + 1);
    }
    // os cabos: uma camada própria, refeita só quando um cabo muda (ganho,
    // condução, ruptura) ou a vista rola — redesenhar 63 cabos atravessando
    // a tela custava ~6 ms por quadro
    const chave = `${topo}|${vw}x${vh}|${this.layoutN}|${this.opacidade}|${this.silenciado}|${this.chaveFixos()}`;
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
    g.setTransform(dpr, 0, 0, dpr, 0, -topo * dpr);
    // módulo à vista por exceção na SAÍDA: borda tracejada
    if (this.soSaida) {
      g.strokeStyle = T.accent; g.lineWidth = 1; g.setLineDash([4, 3]);
      for (const m of this.vistos)
        if (!m.chega && this.pendentes.has(m.id)) g.strokeRect(m.bx + 0.5, m.by + 0.5, m.bw - 1, m.bh - 1);
      g.setLineDash([]);
    }
    this.desenharDestaques(g, topo, vh);
    if (this.gesto?.tipo === 'cabo') this.desenharPuxada(g);
    if (this.fantasma) {   // o módulo vindo da paleta
      g.fillStyle = T.accent; g.font = fonte(11); g.textAlign = 'left'; g.textBaseline = 'middle';
      g.fillText('+ ' + this.fantasma.tipo, this.fantasma.x + 8, this.fantasma.y);
    }
    g.setTransform(dpr, 0, 0, dpr, 0, 0);
    this.desenharInspetor(g, vw, vh);
  }

  // MATRIX: a grade 4×4 como células (`paintMatrix` + `ui::matrixCellMM`);
  // a barra parte do centro: para cima ganho positivo, para baixo negativo
  celula(m, j, k) {
    const cx = 27 + k * 17, cy = 35 + j * 18;
    return { x: m.bx + this.mmpx(cx - 8), y: m.by + this.mmpx(cy - 8.5), w: this.mmpx(16), h: this.mmpx(17) };
  }

  celulaEm(m, p) {
    if (m.tipo !== 'MATRIX') return null;
    for (let j = 0; j < 4; j++)
      for (let k = 0; k < 4; k++)
        if (this.dentro(p, this.celula(m, j, k))) return m.w.find((w) => w.b === `g${j + 1}${k + 1}` && w.v !== undefined) || null;
    return null;
  }

  desenharMatriz(g, m) {
    for (let j = 0; j < 4; j++)
      for (let k = 0; k < 4; k++) {
        const r = this.celula(m, j, k);
        const w = m.w.find((x) => x.b === `g${j + 1}${k + 1}`);
        const v = w ? this.valor(w) : 0;
        g.fillStyle = T.recessed; g.fillRect(r.x, r.y, r.w, r.h);
        const mid = r.y + Math.floor(r.h / 2), bar = Math.trunc(v * (Math.floor(r.h / 2) - 2));
        if (bar !== 0) {
          g.fillStyle = corFamilia(m.familia, v >= 0 ? 1 : 0.45);   // a cor da família, como a faixa
          g.fillRect(r.x + 2, bar > 0 ? mid - bar : mid, r.w - 4, Math.abs(bar));
        }
        g.fillStyle = T.line; g.fillRect(r.x + 1, mid, r.w - 3, 1);
        const quente = this.gesto?.tipo === 'controle' && this.gesto.w === w;
        g.strokeStyle = quente ? T.accent : T.line; g.lineWidth = 1;
        g.strokeRect(r.x + 0.5, r.y + 0.5, r.w - 1, r.h - 1);
      }
  }

  // os pontos de um cabo, para desenho e clique (`ui::cablePoints`)
  pontosCabo(x0, y0, x1, y1) {
    const sag = 18 + Math.abs(x1 - x0) / 6, cx = (x0 + x1) / 2, cy = Math.max(y0, y1) + sag;
    const pts = [];
    for (let k = 0; k < 15; k++) {
      const t = k / 14, u = 1 - t;
      pts.push([u * u * x0 + 2 * u * t * cx + t * t * x1, u * u * y0 + 2 * u * t * cy + t * t * y1]);
    }
    return pts;
  }

  // os cabos à vista como { i, s, t, campos }
  *cabosVisiveis() {
    const c = this.cabos;
    if (!c) return;
    for (let i = 0, k = 0; i + 13 < c.length; i += 14, k++) {
      const s = this.jacks.get(`${c[i]}:${c[i + 1]}:1`), t = this.jacks.get(`${c[i + 2]}:${c[i + 3]}:0`);
      if (s && t) yield { k, i, s, t, c };
    }
  }

  // o cabo mais perto do ponto, até `mmpx(5) + 3` px (`cableUnder`)
  caboEm(p) {
    const tol = this.mmpx(5) + 3;
    let melhor = -1, bestD = tol;
    for (const { k, s, t } of this.cabosVisiveis()) {
      const pts = this.pontosCabo(s.x, s.y, t.x, t.y);
      for (let n = 0; n + 1 < pts.length; n++) {
        const [ax, ay] = pts[n], [bx, by] = pts[n + 1];
        const abx = bx - ax, aby = by - ay, len2 = abx * abx + aby * aby;
        let u = len2 > 1e-6 ? ((p.x - ax) * abx + (p.y - ay) * aby) / len2 : 0;
        u = clamp(u, 0, 1);
        const d = Math.hypot(p.x - (ax + u * abx), p.y - (ay + u * aby));
        if (d <= bestD) { bestD = d; melhor = k; }
      }
    }
    return melhor;
  }

  corCabo(s, c, i) {
    const h = c[i] * 7 + c[i + 1] * 3 + c[i + 2] * 5 + c[i + 3];
    return c[i + 4] ? T.warning : (s.pk === 1 ? CABO_CTRL[h & 3] : CABO_AUDIO[h & 3]);
  }

  // o cabo sob o mouse e o do inspetor: por cima, mais grossos e claros; e
  // o selo das relações (operação no meio do cabo, arco = AMT, pontilhado
  // até o companion)
  desenharDestaques(g, topo, vh) {
    for (const { k, i, s, t, c } of this.cabosVisiveis()) {
      const quente = k === this.sobCabo || (this.insp && c[i + 2] === this.insp.d && c[i + 3] === this.insp.pd);
      if (quente) {
        g.save();
        g.filter = 'brightness(1.45)';
        this.tracarCabo(g, s.x, s.y, t.x, t.y, this.corCabo(s, c, i), !!c[i + 4], 1 + clamp(c[i + 6], 0, 2) + 2);
        g.restore();
      }
      if (c[i + 7]) {
        const pts = this.pontosCabo(s.x, s.y, t.x, t.y);
        const comp = this.jacks.get(`${c[i + 11]}:${c[i + 12]}:1`);
        this.selo(g, pts[7][0], pts[7][1], comp, this.corCabo(s, c, i), c[i + 9], c[i + 10], quente);
      }
    }
  }

  selo(g, x, y, comp, cor, rel, amt, aceso) {
    const r = 9;
    if (comp) {
      g.save();
      g.globalAlpha = aceso ? 0.95 : 0.6;
      g.strokeStyle = cor; g.lineWidth = 1; g.setLineDash([1.5, 3]);
      g.beginPath(); g.moveTo(comp.x, comp.y); g.lineTo(x, y); g.stroke();
      g.restore();
    }
    g.fillStyle = T.surface;
    g.beginPath(); g.arc(x, y, r, 0, 2 * Math.PI); g.fill();
    g.save(); g.globalAlpha = 0.45; g.strokeStyle = cor; g.lineWidth = 1;
    g.beginPath(); g.arc(x, y, r - 0.5, 0, 2 * Math.PI); g.stroke(); g.restore();
    if (amt > 0) {
      g.strokeStyle = cor; g.lineWidth = 2;
      g.beginPath(); g.arc(x, y, r, -Math.PI / 2, -Math.PI / 2 + 2 * Math.PI * clamp(amt, 0, 1)); g.stroke();
    }
    const a = 4;
    g.beginPath();
    if (rel === 1) { g.moveTo(x - a, y - a); g.lineTo(x + a, y + a); g.moveTo(x + a, y - a); g.lineTo(x - a, y + a); }
    else if (rel === 2) { g.moveTo(x - a, y + a * 0.6); g.lineTo(x - a * 0.35, y - a * 0.6); g.lineTo(x + a * 0.35, y + a * 0.6); g.lineTo(x + a, y - a * 0.6); }
    else if (rel === 3) { g.moveTo(x - a, y); g.lineTo(x + a, y); }
    g.strokeStyle = aceso ? T.textPrimary : cor; g.lineWidth = 1.6; g.stroke();
  }

  // o nome de um módulo como no rack ("OSC 2" quando há cópias)
  nomeModulo(id) {
    const m = this.porId.get(id);
    if (!m) return '?';
    return m.w.length && m.w[0].k === K.Label ? m.w[0].l : m.tipo;
  }

  nomePorta(id, p, saida) {
    const m = this.porId.get(id);
    const w = m && m.w.find((x) => x.k === K.Jack && x.p === p && !!x.out === saida);
    return w ? w.b.slice(saida ? 4 : 3) : '';
  }

  // o cabo do inspetor no estado atual (índice no array), ou -1
  caboInspecionado() {
    const c = this.cabos;
    if (!this.insp || !c) return -1;
    for (let i = 0; i + 13 < c.length; i += 14)
      if (c[i + 2] === this.insp.d && c[i + 3] === this.insp.pd) return i;
    return -1;
  }

  // O inspetor (`paintInspector`): canto de baixo à direita da vista
  desenharInspetor(g, vw, vh) {
    this.inspAlvos = [];
    const i = this.caboInspecionado();
    if (i < 0) { if (this.insp && this.cabos) this.insp = null; return; }
    const c = this.cabos;
    const temRel = !!c[i + 7];
    const pad = 8, rowH = 20, bw = 210, rows = temRel ? 8 : 6, bh = pad * 2 + rowH * rows;
    const bx = Math.max(4, vw - bw - 12), by = Math.max(4, vh - bh - 12);
    this.inspCaixa = { x: bx, y: by, w: bw, h: bh };
    g.fillStyle = T.surface; g.fillRect(bx, by, bw, bh);
    g.strokeStyle = T.accent; g.lineWidth = 1; g.strokeRect(bx + 0.5, by + 0.5, bw - 1, bh - 1);
    let ry = by + pad;
    const dot = '  \u00b7  ';
    g.font = fonte(10, true); g.fillStyle = T.textPrimary;
    const tw = bw - pad * 2 - 16;
    const corta = (txt) => { while (txt.length > 1 && g.measureText(txt).width > tw) txt = txt.slice(0, -1); return txt; };
    g.textAlign = 'left'; g.textBaseline = 'middle';
    g.fillText(corta(this.nomeModulo(c[i]) + dot + this.nomePorta(c[i], c[i + 1], true)), bx + pad, ry + 7);
    g.fillText(corta('\u2192 ' + this.nomeModulo(c[i + 2]) + dot + this.nomePorta(c[i + 2], c[i + 3], false)), bx + pad, ry + rowH + 3);
    const lx = bx + bw - pad - 9;
    g.fillStyle = c[i + 5] && !c[i + 4] ? T.accent : T.recessed;
    g.beginPath(); g.arc(lx + 4, ry + 7, 4, 0, 2 * Math.PI); g.fill();
    g.strokeStyle = T.line; g.stroke();
    if (c[i + 13]) {
      g.font = fonte(8); g.fillStyle = T.warning; g.textAlign = 'right';
      g.fillText('\u21ba ' + this.tx('inspFeedback'), bx + bw - pad, ry + 2 * rowH - 3);
      g.textAlign = 'left';
    }
    ry += 2 * rowH;
    const botao = (x, y, w, h, txt, ativo, acao) => {
      if (ativo) { g.fillStyle = T.accent; g.fillRect(x, y, w, h); }
      g.strokeStyle = ativo ? T.accent : T.line; g.lineWidth = 1; g.strokeRect(x + 0.5, y + 0.5, w - 1, h - 1);
      g.fillStyle = ativo ? T.bg : T.textSecondary; g.font = fonte(9);
      textoAjustado(g, txt, x + 2, y, w - 4, h);
      this.inspAlvos.push({ x, y, w, h, acao });
    };
    const rel = c[i + 9];
    const relW = Math.floor((bw - pad * 2) / 4);
    let cx = bx + pad;
    [['NONE', 0], ['RING', 1], ['FOLD', 2], ['DIFF', 3]].forEach(([txt, r], n) => {
      const w = n < 3 ? relW : bw - pad - cx + bx;
      botao(cx, ry, w, rowH - 2, txt, rel === r, { tipo: 'rel', r });
      cx += relW;
    });
    ry += rowH;
    const trilha = (txt, v, acao, meio) => {
      g.fillStyle = T.textSecondary; g.font = fonte(9);
      textoAjustado(g, txt, bx + pad, ry, 40, rowH - 2, 'esquerda');
      const tx = bx + pad + 42, tw2 = bw - pad * 2 - 42;
      g.strokeStyle = T.line; g.strokeRect(tx + 0.5, ry + 3.5, tw2 - 1, rowH - 9);
      const fw = Math.floor(tw2 * clamp(v, 0, 1));
      if (fw > 0) { g.fillStyle = T.accent; g.fillRect(tx, ry + 3, fw, rowH - 8); }
      if (meio) {
        g.strokeStyle = 'rgba(190, 200, 216, 0.8)';
        g.beginPath(); g.moveTo(tx + Math.floor(tw2 / 2) + 0.5, ry + 4); g.lineTo(tx + Math.floor(tw2 / 2) + 0.5, ry + rowH - 6); g.stroke();
      }
      this.inspAlvos.push({ x: tx, y: ry, w: tw2, h: rowH - 2, acao, trilha: true });
      ry += rowH;
    };
    if (temRel) {
      const cr = { x: bx + pad, y: ry, w: bw - pad * 2, h: rowH - 2 };
      g.strokeStyle = T.line; g.strokeRect(cr.x + 0.5, cr.y + 0.5, cr.w - 1, cr.h - 1);
      g.fillStyle = this.escolhendo ? T.accent : T.textSecondary; g.font = fonte(9);
      textoAjustado(g, 'COMP  ' + this.nomeModulo(c[i + 11]) + dot + this.nomePorta(c[i + 11], c[i + 12], true), cr.x + 4, cr.y, cr.w - 8, cr.h, 'esquerda');
      this.inspAlvos.push({ ...cr, acao: { tipo: 'comp' } });
      ry += rowH;
      trilha('AMT', c[i + 10], { tipo: 'amt' }, false);
    }
    trilha('GAIN', c[i + 6] * 0.5, { tipo: 'ganho' }, true);
    trilha('COND', c[i + 8], { tipo: 'cond' }, false);
    const metade = Math.floor((bw - pad * 2 - 4) / 2);
    botao(bx + pad, ry, metade, rowH - 2, this.tx(c[i + 4] ? 'inspReconnect' : 'inspRupture'), !!c[i + 4], { tipo: 'romper' });
    botao(bx + pad + metade + 4, ry, metade, rowH - 2, this.tx('inspRemove'), false, { tipo: 'tirar' });
  }

  // um clique no inspetor; true = o clique era dele
  cliqueInspetor(e) {
    const r = this.cv.getBoundingClientRect();
    const x = e.clientX - r.left, y = e.clientY - r.top;
    const alvo = (this.inspAlvos || []).find((a) => x >= a.x && x < a.x + a.w && y >= a.y && y < a.y + a.h);
    const cx = this.inspCaixa;
    if (!alvo) {
      if (cx && x >= cx.x && x < cx.x + cx.w && y >= cx.y && y < cx.y + cx.h) return true;
      this.insp = null; this.escolhendo = false; this.redesenhar();
      return false;                     // fora: fecha, e o clique segue
    }
    const i = this.caboInspecionado();
    if (i < 0) return true;
    const c = this.cabos, d = this.insp.d, pd = this.insp.pd, a = alvo.acao;
    // relação, condutância, ganho e ruptura são desfazíveis (a foto do
    // DESFAZ antes do gesto; tirar o cabo já fotografa no motor)
    if (a.tipo !== 'comp' && a.tipo !== 'tirar') this.enviar({ tipo: 'foto' });
    if (a.tipo === 'rel') {
      if (a.r === 0) this.enviar({ tipo: 'caboRelacao', d, pd, rel: 0 });
      else if (!c[i + 7]) {             // relação nova: companion = a própria origem, e escolher
        this.enviar({ tipo: 'caboRelacao', d, pd, rel: a.r, cn: c[i], cp: c[i + 1], amt: 0.5 });
        this.escolhendo = true;
      } else this.enviar({ tipo: 'caboRelacao', d, pd, rel: a.r, cn: c[i + 11], cp: c[i + 12], amt: c[i + 10] });
    } else if (a.tipo === 'comp') this.escolhendo = true;
    else if (a.tipo === 'romper') this.enviar({ tipo: 'caboRomper', d, pd });
    else if (a.tipo === 'tirar') { this.enviar({ tipo: 'cortarEntrada', d, pd }); this.insp = null; }
    else if (alvo.trilha) {
      this.cv.setPointerCapture(e.pointerId);
      this.gesto = { tipo: 'trilha', acao: a.tipo, x0: alvo.x + r.left, w: alvo.w, comp: [c[i + 9], c[i + 11], c[i + 12]] };
      this.trilha(e);
    }
    this.redesenhar();
    return true;
  }

  // arrastando AMT / GAIN / COND (`applyCableSlider`)
  trilha(e) {
    const gs = this.gesto, d = this.insp?.d, pd = this.insp?.pd;
    if (d === undefined) return;
    const v = clamp((e.clientX - gs.x0) / Math.max(1, gs.w), 0, 1);
    if (gs.acao === 'amt') this.enviar({ tipo: 'caboRelacao', d, pd, rel: gs.comp[0], cn: gs.comp[1], cp: gs.comp[2], amt: v });
    else if (gs.acao === 'cond') this.enviar({ tipo: 'caboCondutancia', d, pd, v });
    else this.enviar({ tipo: 'caboGanho', d, pd, v: v * 2 });
  }

  // Puxando um cabo (`paintWidget`/Jack + `paintCables` com `cdrag_`): os
  // destinos de polaridade oposta acendem — anel duplo se o tipo de sinal
  // casa, simples se cruza domínio; contínuo se o destino chega ao som,
  // tracejado se ainda não. Os inválidos recuam.
  desenharPuxada(g) {
    const c = this.gesto;
    const r = this.mmpx(2.4);
    for (const [, j] of this.jacks) {
      if (!!j.out === c.deSaida) {   // mesma polaridade: recua (j.out é 0/1, deSaida é booleano)
        g.strokeStyle = 'rgba(72, 80, 96, 0.55)'; g.lineWidth = 1.5;
        g.beginPath(); g.arc(j.x, j.y, r - 0.75, 0, 2 * Math.PI); g.stroke();
        continue;
      }
      const forte = j.pk === c.pk;
      const soa = !!this.porId.get(j.no)?.chega;
      const hr = r + this.mmpx(forte ? 2.4 : 1.6);
      const anel = (rad) => {
        g.lineWidth = soa ? 1.8 : 1.2;
        g.strokeStyle = soa ? T.accent : 'rgba(216, 221, 230, 0.55)';
        g.setLineDash(soa ? [] : [3, 3]);
        g.beginPath(); g.arc(j.x, j.y, rad, 0, 2 * Math.PI); g.stroke();
      };
      anel(hr);
      if (forte) anel(hr + 2);
      g.setLineDash([]);
      if (soa) { g.fillStyle = 'rgba(216, 221, 230, 0.28)'; g.beginPath(); g.arc(j.x, j.y, r, 0, 2 * Math.PI); g.fill(); }
      g.strokeStyle = T.accent; g.lineWidth = 1.5;
      g.beginPath(); g.arc(j.x, j.y, r - 0.75, 0, 2 * Math.PI); g.stroke();
    }
    const cor = c.pk === 1 ? CABO_CTRL[0] : CABO_AUDIO[0];
    // fonte sem sinal agora: o cabo sai acinzentado (`dragSourceSilent_`) —
    // explica o "liguei e nada aconteceu" antes de ligar
    g.globalAlpha = c.mudo ? 0.3 : 1;
    this.tracarCabo(g, c.ax, c.ay, this.mouse.x, this.mouse.y, cor, true);
    g.globalAlpha = 1;
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
    // a área das saídas (`ui::outputPlates`)
    if (m.pl) {
      g.fillStyle = T.placa;
      for (let k = 0; k + 3 < m.pl.length; k += 4) {
        g.beginPath();
        g.roundRect(m.bx + this.mmpx(m.pl[k]), m.by + this.mmpx(m.pl[k + 1]),
                    this.mmpx(m.pl[k + 2]), this.mmpx(m.pl[k + 3]), this.mmpx(1.5));
        g.fill();
      }
    }
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
      if (w.k === K.Jack && w.li !== undefined) {   // o LED apagado
        g.fillStyle = T.recessed;
        g.beginPath(); g.arc(wx + this.mmpx(LUZ.dx), wy + this.mmpx(LUZ.dy), this.mmpx(LUZ.r), 0, 2 * Math.PI); g.fill();
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

  // o wordmark: 22 mm, na base; o lugar vem do motor (`ui::wordmarkXMM`)
  wordmark(g, m) {
    if (!this.marca || !(m.wm >= 0)) return;
    const lw = this.mmpx(22);
    const lh = lw * WM.h / WM.w;
    const topo = m.by + this.mmpx(MM_3U - 2.5) - lh;
    const x = m.bx + this.mmpx(m.wm);
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
        if (w.pr) {   // o controle principal (`ui::primaryControl`): escala e corpo invertido
          g.strokeStyle = T.textSecondary; g.lineWidth = 1;
          g.beginPath();
          for (let k = 0; k < 11; k++) {
            const a = (-135 + 27 * k) * Math.PI / 180, s = Math.sin(a), c = -Math.cos(a);
            const r0 = r + this.mmpx(0.5), r1 = r + this.mmpx(k % 5 === 0 ? 1.5 : 1.2);
            g.moveTo(cx + s * r0, cy + c * r0); g.lineTo(cx + s * r1, cy + c * r1);
          }
          g.stroke();
          const gp = g.createLinearGradient(0, wy, 0, wy + 2 * r);
          gp.addColorStop(0, '#eef1f6'); gp.addColorStop(1, '#aab2c0');
          g.fillStyle = gp;
          g.beginPath(); g.arc(cx, cy, r, 0, 2 * Math.PI); g.fill();
          g.strokeStyle = T.line; g.lineWidth = 1;
          g.beginPath(); g.arc(cx, cy, r - 0.5, 0, 2 * Math.PI); g.stroke();
          g.fillStyle = T.textSecondary;
          this.rotulo(g, m, w.l, wx - this.mmpx(2), wy - this.mmpx(4.6), 2 * r + this.mmpx(4), this.mmpx(3.5));
          break;
        }
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
        if (w.rd) {   // visor redondo (`ui::roundDisplay`): o círculo inscrito
          const d = Math.min(dw, dh);
          g.fillStyle = T.recessed; g.beginPath(); g.arc(wx + dw / 2, wy + dh / 2, d / 2, 0, 2 * Math.PI); g.fill();
          g.strokeStyle = T.line; g.lineWidth = 1; g.beginPath(); g.arc(wx + dw / 2, wy + dh / 2, d / 2 - 0.5, 0, 2 * Math.PI); g.stroke();
          break;
        }
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

  // ---- a mão no instrumento (`RackView::mouseDown/Drag/Up`) -----------
  ponto(e) {
    const r = this.cv.getBoundingClientRect();
    return { x: e.clientX - r.left, y: e.clientY - r.top + this.rolagem.scrollTop };
  }

  jackEm(p) {
    const r = this.mmpx(3.2) + 2;
    for (const [, j] of this.jacks) {
      const dx = p.x - j.x, dy = p.y - j.y;
      if (dx * dx + dy * dy <= r * r) return j;
    }
    return null;
  }

  moduloEm(p) {
    for (const m of this.vistos)
      if (p.x >= m.bx && p.x < m.bx + m.bw && p.y >= m.by && p.y < m.by + m.bh) return m;
    return null;
  }

  // a fonte do cabo puxado está em silêncio agora? (o pico do visor dela)
  fonteMuda(no) {
    const v = this.visores.get(no);
    if (!v) return false;   // sem telemetria (fora da tela, sem visor): não acusa
    let p = 0;
    for (let k = 0; k < 220; k++) p = Math.max(p, Math.abs(v[k]));
    return p < 1e-4;
  }

  // o [x] (`removeBox`) e a faixa do nome, a pega do módulo (`titleStrip`)
  caixaX(m) { return { x: m.bx + m.bw - 15, y: m.by + 2, w: 13, h: 13 }; }
  faixaTitulo(m) { return { x: m.bx, y: m.by, w: m.bw - 17, h: this.mmpx(6) }; }

  // `reorderTo`: a fileira e o lado do cursor dizem onde o módulo entra,
  // contado entre os visíveis e traduzido para a ordem inteira; no máximo
  // ~25 por segundo
  reordenar(id, p) {
    const agora = performance.now();
    if (agora - this.ultimaOrdem < 40) return;
    this.ultimaOrdem = agora;
    const linhaH = this.modH + PAD;
    const linha = Math.max(0, Math.floor((p.y - TOPO) / linhaH));
    const vis = this.vistos.filter((m) => m.id !== id);
    let i = 0;
    for (const m of vis) {
      const ml = Math.floor((m.by - TOPO) / linhaH);
      if (ml < linha || (ml === linha && m.bx + m.bw / 2 < p.x)) i++;
    }
    const prox = this.mods.filter((m) => m.id !== id);
    const pos = (mm) => prox.indexOf(mm);
    let at;
    if (!vis.length) at = prox.length;
    else if (i <= 0) at = pos(vis[0]);
    else if (i >= vis.length) at = pos(vis[vis.length - 1]) + 1;
    else at = pos(vis[i]);
    at = clamp(at, 0, prox.length);
    prox.splice(at, 0, this.porId.get(id));
    if (prox.some((m, k) => m !== this.mods[k])) { this.mods = prox; this.layout(); }
  }

  // arrastando perto da borda de cima/de baixo, o rack rola
  rolarNaBorda(e) {
    const r = this.rolagem.getBoundingClientRect();
    if (e.clientY < r.top + 30) this.rolagem.scrollTop -= 14;
    else if (e.clientY > r.bottom - 30) this.rolagem.scrollTop += 14;
  }

  // LEARN (`learnAt`): o que está sob o ponto, na prioridade do clique —
  // 1. widget (knob, slider, botão, jack, célula), 2. cabo, 3. corpo do
  // módulo. Durante um arrasto, o controle arrastado continua sendo o alvo.
  learnEm(p) {
    const widget = (m, w) => ({ chave: `${m.id}|${w.b}`, tipo: m.tipo, bind: w.b, no: m.id,
                                titulo: `${m.tipo}  ·  ${w.l}`, param: w.v !== undefined });
    if (this.gesto?.tipo === 'controle') {
      const m = this.porId.get(this.gesto.no);
      if (m) return widget(m, this.gesto.w);
    }
    const m = this.moduloEm(p);
    if (m) {
      const j = this.jackEm(p);
      if (j && j.no === m.id) {
        const w = m.w.find((x) => x.k === K.Jack && x.p === j.p && !!x.out === !!j.out);
        if (w?.b) return widget(m, w);
      }
      const c = this.celulaEm(m, p);
      if (c) return widget(m, c);
      for (const w of m.w) {
        if (!w.b || w.k === K.Jack || (m.tipo === 'MATRIX' && celulaMatriz(w.b))) continue;
        if (this.dentro(p, this.caixa(m, w))) return widget(m, w);
      }
    }
    const k = this.caboEm(p);
    if (k >= 0) return { chave: 'cabo|' + k, cabo: true };
    if (m) return { chave: `${m.id}|\u0001mod`, tipo: m.tipo, modulo: true };
    return null;
  }

  // a área de toque de um controle (`ui::footprintMM`)
  caixa(m, w) {
    const lbl = w.l.length;
    let r;
    if (w.k === K.Knob) { const d = diamKnob(w); r = [w.x - 0.5, w.y - 1, d + 2, d + 7]; }
    else if (w.k === K.Slider) r = [w.x - 1, w.y - 1, 10, 36];
    else if (w.k === K.Toggle) r = w.s > 1 ? [w.x + 2 - w.s * 0.5 - 0.5, w.y - 0.5, Math.max(w.s, 4) + 1, 9] : [w.x - 0.5, w.y - 0.5, 5 + lbl * 1.7, 6];
    else if (w.k === K.Display) r = [w.x, w.y, Math.min(w.s > 1 ? w.s : 16, m.hp * MM_HP - w.x - 2), w.h];
    else return null;
    return { x: m.bx + this.mmpx(r[0]), y: m.by + this.mmpx(r[1]), w: this.mmpx(r[2]), h: this.mmpx(r[3]) };
  }

  dentro(p, c) { return c && p.x >= c.x && p.x < c.x + c.w && p.y >= c.y && p.y < c.y + c.h; }

  // o knob/slider sob o ponto ou o mais perto até `tol` mm (`nearestControl`)
  controlePerto(m, p, tol) {
    let melhor = null, bestD = this.mmpx(tol) + 1;
    for (const w of m.w) {
      if ((w.k !== K.Knob && w.k !== K.Slider) || w.v === undefined) continue;
      if (m.tipo === 'MATRIX' && celulaMatriz(w.b)) continue;
      const c = this.caixa(m, w);
      const d = Math.max(Math.max(c.x - p.x, 0, p.x - (c.x + c.w)), Math.max(c.y - p.y, 0, p.y - (c.y + c.h)));
      if (d < bestD) { bestD = d; melhor = w; }
    }
    return melhor;
  }

  // o controle do ponto, para pegar: toggle, knob ou slider
  controleEm(m, p) {
    for (const w of m.w) {
      if (w.v === undefined || (w.k !== K.Knob && w.k !== K.Slider && w.k !== K.Toggle)) continue;
      if (m.tipo === 'MATRIX' && w.k === K.Knob && celulaMatriz(w.b)) continue;
      if (this.dentro(p, this.caixa(m, w))) return w;
    }
    return this.controlePerto(m, p, 4);
  }

  baixa(e) {
    // o botão do meio arrasta o rack para cima e para baixo (`pan_` do app),
    // também enquanto se cabeia
    if (e.button === 1) {
      e.preventDefault();
      this.panorama = { y0: e.clientY, topo0: this.rolagem.scrollTop };
      this.cv.setPointerCapture(e.pointerId);
      return;
    }
    const p = this.ponto(e);
    this.mouse = p;
    // escolhendo o companion: o próximo clique numa saída o define
    if (this.escolhendo) {
      const j = this.jackEm(p), i = this.caboInspecionado();
      if (j && j.out && i >= 0) {
        const c = this.cabos;
        this.enviar({ tipo: 'foto' });
        this.enviar({ tipo: 'caboRelacao', d: this.insp.d, pd: this.insp.pd, rel: c[i + 9], cn: j.no, cp: j.p, amt: c[i + 10] });
      }
      this.escolhendo = false;
      this.redesenhar();
      return;
    }
    if (this.insp && e.button === 0 && this.cliqueInspetor(e)) return;
    const j = this.jackEm(p);
    if (j) {
      if (e.button === 2) {                      // direito: corta
        if (j.out) this.enviar({ tipo: 'cortarSaida', o: j.no, po: j.p });
        else this.enviar({ tipo: 'cortarEntrada', d: j.no, pd: j.p });
        return;
      }
      this.cv.setPointerCapture(e.pointerId);
      // entrada com cabo: puxa o cabo dali, a partir da origem dele
      if (!j.out && this.cabos) {
        const c = this.cabos;
        for (let i = 0; i + 13 < c.length; i += 14)
          if (c[i + 2] === j.no && c[i + 3] === j.p) {
            const s = this.jacks.get(`${c[i]}:${c[i + 1]}:1`);
            this.enviar({ tipo: 'cortarEntrada', d: j.no, pd: j.p });
            this.gesto = { tipo: 'cabo', no: c[i], p: c[i + 1], deSaida: true, pk: s ? s.pk : 0,
                           ax: s ? s.x : p.x, ay: s ? s.y : p.y, mudo: this.fonteMuda(c[i]) };
            this.enviar({ tipo: 'mao', ativa: true });
            this.redesenhar();
            return;
          }
      }
      this.gesto = { tipo: 'cabo', no: j.no, p: j.p, deSaida: !!j.out, pk: j.pk, ax: j.x, ay: j.y,
                     mudo: !!j.out && this.fonteMuda(j.no) };
      this.enviar({ tipo: 'mao', ativa: true });
      this.redesenhar();
      return;
    }
    if (e.button !== 0) return;
    const m = this.moduloEm(p);
    // o [x]: tira o módulo ao soltar, se o mouse não escorregou
    if (m && this.dentro(p, this.caixaX(m))) {
      this.gesto = { tipo: 'x', id: m.id, x0: e.clientX, y0: e.clientY };
      return;
    }
    const w = m && (this.celulaEm(m, p) || this.controleEm(m, p));
    // QUANTIZER: clicar no visor abre o banco de escalas
    if (!w && m?.tipo === 'QUANTIZER' && m.w.some((x) => x.k === K.Display && this.dentro(p, this.caixa(m, x)))) {
      this.abrirBanco(m.id);
      return;
    }
    // SCOPE: clicar no visor alterna onda ↔ espectro
    if (!w && m?.tipo === 'SCOPE' && m.w.some((x) => x.k === K.Display && this.dentro(p, this.caixa(m, x)))) {
      if (!this.espectro.delete(m.id)) this.espectro.add(m.id);
      this.redesenhar();
      return;
    }
    if (!w) {
      // um clique (sem arrastar) num cabo abre o inspetor dele
      // clique sobre um cabo é sempre do cabo (regra do autor, 2 out. 2026)
      const k = this.caboEm(p);
      if (k >= 0) { this.gesto = { tipo: 'clique-cabo', k, x0: e.clientX, y0: e.clientY }; return; }
      // o módulo só se move pela faixa do nome
      if (m && this.dentro(p, this.faixaTitulo(m))) {
        this.cv.setPointerCapture(e.pointerId);
        this.gesto = { tipo: 'modulo', id: m.id, x0: e.clientX, y0: e.clientY, movido: false };
      }
      return;
    }
    if (w.k === K.Toggle) {
      this.enviar({ tipo: 'alterna', i: w.v });
      if (this.valores) { this.valores[w.v] = this.valores[w.v] >= 0.5 ? 0 : 1; this.redesenhar(); }
      return;
    }
    this.cv.setPointerCapture(e.pointerId);
    this.gesto = { tipo: 'controle', w, no: m.id, y0: e.clientY };
    this.enviar({ tipo: 'pega', i: w.v });
  }

  move(e) {
    const p = this.ponto(e);
    this.mouse = p;
    const gs = this.gesto;
    if (gs?.tipo === 'controle') {
      this.enviar({ tipo: 'arrasto', i: gs.w.v, dy: (gs.y0 - e.clientY) / 220, lo: gs.w.min, hi: gs.w.max });
      return;
    }
    if (this.panorama) { this.rolagem.scrollTop = this.panorama.topo0 + (this.panorama.y0 - e.clientY); return; }
    if (gs?.tipo === 'cabo') { this.rolarNaBorda(e); this.redesenhar(); return; }
    if (gs?.tipo === 'trilha') { this.trilha(e); return; }
    if (gs?.tipo === 'clique-cabo' || gs?.tipo === 'x') {
      if (Math.hypot(e.clientX - gs.x0, e.clientY - gs.y0) > 6) this.gesto = null;
      return;
    }
    if (gs?.tipo === 'modulo') {
      if (!gs.movido) {                // zona morta de 6 px: ainda é clique
        if (Math.hypot(e.clientX - gs.x0, e.clientY - gs.y0) <= 6) return;
        gs.movido = true;
        this.enviar({ tipo: 'foto' });   // desfazer devolve o módulo ao lugar
      }
      this.rolarNaBorda(e);
      this.reordenar(gs.id, this.ponto(e));
      this.redesenhar();
      return;
    }
    // o cursor diz o que dá para pegar; fora de controle, o cabo acende
    let pega = !!this.jackEm(p), mover = false;
    if (!pega) {
      const m = this.moduloEm(p);
      pega = !!(m && (this.celulaEm(m, p) || this.controleEm(m, p) || this.dentro(p, this.caixaX(m))));
      mover = !pega && !!m && this.dentro(p, this.faixaTitulo(m));
    }
    const sob = pega ? -1 : this.caboEm(p);
    if (sob !== this.sobCabo) { this.sobCabo = sob; this.redesenhar(); }
    this.cv.style.cursor = pega || sob >= 0 ? 'pointer' : mover ? 'grab' : 'default';
  }

  sobe(e) {
    if (this.panorama && e.button === 1) { this.panorama = null; return; }
    const gs = this.gesto;
    this.gesto = null;
    if (!gs) return;
    if (gs.tipo === 'controle') { this.enviar({ tipo: 'solta' }); return; }
    if (gs.tipo === 'trilha') return;
    if (gs.tipo === 'x') {
      const m = this.porId.get(gs.id);
      if (m && this.dentro(this.ponto(e), this.caixaX(m)) && Math.hypot(e.clientX - gs.x0, e.clientY - gs.y0) <= 6)
        this.enviar({ tipo: 'remover', id: gs.id });
      return;
    }
    if (gs.tipo === 'modulo') {
      // solto sobre a paleta: sai da case; senão a ordem nova vai ao motor
      if (gs.movido && this.sobrePaleta(e)) this.enviar({ tipo: 'remover', id: gs.id, foto: false });
      else if (gs.movido) this.enviar({ tipo: 'ordem', ids: this.mods.map((m) => m.id) });
      this.redesenhar();
      return;
    }
    if (gs.tipo === 'clique-cabo') {
      const c = this.cabos;
      for (const v of this.cabosVisiveis())
        if (v.k === gs.k) this.insp = { d: c[v.i + 2], pd: c[v.i + 3] };
      this.redesenhar();
      return;
    }
    this.enviar({ tipo: 'mao', ativa: false });
    const j = this.jackEm(this.ponto(e));
    if (j && !!j.out !== gs.deSaida) {
      const [o, po, d, pd] = gs.deSaida ? [gs.no, gs.p, j.no, j.p] : [j.no, j.p, gs.no, gs.p];
      this.enviar({ tipo: 'ligar', o, po, d, pd });
      if (!this.porId.get(d)?.chega)
        this.aviso(this.tx('cableNotAudible'));
    }
    this.redesenhar();
  }

  cancela() {
    if (this.gesto?.tipo === 'controle') this.enviar({ tipo: 'solta' });
    else if (this.gesto) this.enviar({ tipo: 'mao', ativa: false });
    this.gesto = null;
    this.redesenhar();
  }

  // a roda sobre um knob/slider gira o controle; fora dele, rola o rack
  roda(e) {
    if (!e.deltaY) return;
    const p = this.ponto(e);
    const m = this.moduloEm(p);
    const w = m && this.controlePerto(m, p, 2);
    if (!w) return;
    e.preventDefault();
    this.enviar({ tipo: 'roda', i: w.v, dir: e.deltaY < 0 ? 1 : -1, fino: e.shiftKey, lo: w.min, hi: w.max });
  }

  // ---- cabos (`paintCables` + `strokeCable`) --------------------------
  // `fixos`: true = só os cabos de condução 1 (a camada guardada), false =
  // só os que piscam (redesenhados a cada quadro)
  desenharCabos(g, topo, vh, fixos) {
    const c = this.cabos;
    if (!c) return;
    for (let i = 0; i + 13 < c.length; i += 14) {
      const [sn, sp, tn, tp, rompido, conduz, ganho, , condut] = c.subarray(i, i + 9);
      if ((condut >= 1) !== fixos) continue;
      const s = this.jacks.get(`${sn}:${sp}:1`), t = this.jacks.get(`${tn}:${tp}:0`);
      if (!s || !t) continue;   // uma das pontas é o sink (não exibido)
      const sag = 18 + Math.abs(t.x - s.x) / 6, baixo = Math.max(s.y, t.y) + sag;
      if (Math.min(s.y, t.y) > topo + vh || baixo < topo) continue;
      const h = sn * 7 + sp * 3 + tn * 5 + tp;
      const cor = rompido ? T.warning : (s.pk === 1 ? CABO_CTRL[h & 3] : CABO_AUDIO[h & 3]);
      g.globalAlpha = this.opacidade * (!rompido && !conduz ? 0.22 : 1) * (this.silenciado ? 0.32 : 1);
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
    for (let i = 0; i + 13 < c.length; i += 14)
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
