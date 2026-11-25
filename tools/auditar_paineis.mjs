// Auditoria de diagramação dos painéis (25/11/2026; pedido do autor: "auditar
// os módulos para verificar o melhor jeito de posicionar os objetos, tanto
// no modo normal como no modo compacto").
//
// Lê os painéis do próprio motor (o .wasm do navegador: `rw_descrever`, com
// e sem `rw_compacto`) e mede, para cada tipo de módulo, nos dois modos:
//   colunas   objetos de fileiras diferentes fora das colunas uns dos outros
//   passo     irregularidade dos vãos dentro de cada fileira (mm)
//   centro    deslocamento do conteúdo em relação ao centro do painel (mm)
//   vazio     maior faixa vertical sem nenhum objeto (mm)
//   aperto    pares de vizinhos (objeto ou rótulo) a menos de 1,5 mm
//   visor     visor mais estreito que o painel (mm a menos)
//   uso       % da largura do painel que o conteúdo ocupa
// e soma uma nota de problemas (0 = nada a apontar). Uso:
//   node tools/auditar_paineis.mjs .local-build/web/rasgo.wasm [saida.json]
// Imprime a tabela, do pior para o melhor, e grava o JSON se pedido.

import fs from 'fs';

const [wasmPath, saida] = process.argv.slice(2);
const bytes = fs.readFileSync(wasmPath || '.local-build/web/rasgo.wasm');
let mem;
const stub = new Proxy({}, { get: (_, n) => (...a) => {
  if (n === 'fd_write') { const v = new DataView(mem.buffer); let s = 0; for (let k = 0; k < a[2]; k++) s += v.getUint32(a[1] + 8 * k + 4, true); v.setUint32(a[3], s, true); }
  return 0; } });
const { instance } = await WebAssembly.instantiate(bytes, { wasi_snapshot_preview1: stub, env: stub });
const e = instance.exports; mem = e.memory; if (e._initialize) e._initialize();
e.rw_iniciar(48000); e.rw_seed(1, 2);
const descrever = () => JSON.parse(new TextDecoder().decode(new Uint8Array(mem.buffer, e.rw_descrever(), e.rw_texto_tamanho())));

const K = { Knob: 0, Slider: 1, Toggle: 2, Jack: 3, Display: 4, Label: 5 };
const MM_HP = 5.08;

// o centro e a meia-largura de cada objeto (com o rótulo, ~1,55 mm por letra),
// e a extensão vertical (rótulo em cima incluído)
function objetos(m) {
  const r = [];
  for (const w of m.w) {
    if (w.k === K.Label || w.k === K.Display) continue;
    let cx = w.x, cy = w.y, meia = 3, topo, fundo;
    const rot = 0.5 * (1.55 * (w.l || '').length + 1);
    if (w.k === K.Knob) { const d = w.s > 1 ? w.s : 8; cx += d / 2; cy += d / 2; meia = Math.max(d / 2, rot); topo = w.y - 4; fundo = w.y + d; }
    else if (w.k === K.Slider) { cx += 3; cy += 15; meia = Math.max(3, rot); topo = w.y; fundo = w.y + 34; }
    else if (w.k === K.Toggle) { cx += 2; cy += 2; meia = Math.max(3, rot); topo = w.y; fundo = w.y + 6; }
    else { meia = Math.max(4.2, rot); topo = w.y - 7; fundo = w.y + 3.2; }   // jack, rótulo em cima
    r.push({ w, cx, cy, meia, topo, fundo });
  }
  return r;
}

function fileiras(obj) {
  const fs_ = [];
  for (const o of [...obj].sort((a, b) => a.cy - b.cy)) {
    const f = fs_.find((x) => Math.abs(x.cy - o.cy) < 4);
    if (f) { f.itens.push(o); f.cy = (f.cy * (f.itens.length - 1) + o.cy) / f.itens.length; }
    else fs_.push({ cy: o.cy, itens: [o] });
  }
  for (const f of fs_) f.itens.sort((a, b) => a.cx - b.cx);
  return fs_;
}

function auditar(m) {
  const W = m.hp * MM_HP;
  const obj = objetos(m);
  if (obj.length < 2) return null;
  const fil = fileiras(obj);
  // colunas: um objeto está "na coluna" se outro objeto de OUTRA fileira tem
  // o centro a menos de 1 mm; os que não estão, numa fileira de 2 ou mais,
  // contam (os sozinhos de cada fileira centrados também contam como ok)
  let foraColuna = 0;
  for (const f of fil) for (const o of f.itens) {
    if (fil.length < 2) break;
    const alinhado = fil.some((g) => g !== f && g.itens.some((p) => Math.abs(p.cx - o.cx) < 1));
    const centrado = Math.abs(o.cx - W / 2) < 1;
    if (!alinhado && !centrado) foraColuna++;
  }
  // passo: desvio máximo dos vãos de uma fileira em relação à média dela
  let passo = 0;
  for (const f of fil) {
    if (f.itens.length < 3) continue;
    const gaps = f.itens.slice(1).map((o, k) => o.cx - f.itens[k].cx);
    const med = gaps.reduce((a, b) => a + b, 0) / gaps.length;
    passo = Math.max(passo, ...gaps.map((g) => Math.abs(g - med)));
  }
  // centro: o meio do conteúdo contra o meio do painel
  const xmin = Math.min(...obj.map((o) => o.cx - o.meia)), xmax = Math.max(...obj.map((o) => o.cx + o.meia));
  const centro = Math.abs((xmin + xmax) / 2 - W / 2);
  // vazio: maior faixa vertical sem objeto, entre o fim do visor/título e o
  // último objeto
  const disp = m.w.find((w) => w.k === K.Display);
  let y = disp ? disp.y + (disp.h || 16) : 8;
  let vazio = 0;
  for (const o of [...obj].sort((a, b) => a.topo - b.topo)) { vazio = Math.max(vazio, o.topo - y); y = Math.max(y, o.fundo); }
  // aperto: vizinhos na mesma fileira a menos de 1,5 mm (objeto ou rótulo)
  let aperto = 0;
  for (const f of fil) for (let k = 1; k < f.itens.length; k++) {
    const a = f.itens[k - 1], b = f.itens[k];
    if (b.cx - b.meia - (a.cx + a.meia) < 1.5 - 1e-6) aperto++;
  }
  const visor = disp ? Math.max(0, (W - 5) - (disp.s > 1 ? disp.s : 16)) : 0;
  const uso = Math.round(100 * (xmax - xmin) / W);
  // a nota: cada medida pesa o que costuma pesar no olho
  const nota = Math.round(foraColuna * 2 + passo * 0.8 + centro * 0.8 + Math.max(0, vazio - 12) * 0.5
                          + aperto * 3 + visor * 0.3 + Math.max(0, 80 - uso) * 0.3);
  return { hp: m.hp, foraColuna, passo: +passo.toFixed(1), centro: +centro.toFixed(1), vazio: Math.round(vazio),
           aperto, visor: Math.round(visor), uso, nota };
}

const res = {};
for (const [modo, liga] of [['normal', 0], ['compacto', 1]]) {
  e.rw_compacto(liga);
  for (const m of descrever().modulos) {
    if (m.inst > 1 && res[m.tipo]) continue;
    const a = auditar(m);
    if (!a) continue;
    (res[m.tipo] ||= { tipo: m.tipo, familia: m.familia })[modo] = a;
  }
}
const linhas = Object.values(res).sort((a, b) => (b.normal.nota + b.compacto.nota) - (a.normal.nota + a.compacto.nota));
const cel = (a) => `${String(a.hp).padStart(2)}HP n${String(a.nota).padStart(3)} col${String(a.foraColuna).padStart(2)} pas${String(a.passo).padStart(5)} ctr${String(a.centro).padStart(5)} vaz${String(a.vazio).padStart(3)} apt${String(a.aperto).padStart(2)} vis${String(a.visor).padStart(3)} uso${String(a.uso).padStart(4)}%`;
console.log('tipo          família    | NORMAL' + ' '.repeat(62) + '| COMPACTO');
for (const l of linhas) console.log(`${l.tipo.padEnd(13)} ${l.familia.padEnd(10)} | ${cel(l.normal)} | ${cel(l.compacto)}`);
if (saida) fs.writeFileSync(saida, JSON.stringify(linhas, null, 1));
