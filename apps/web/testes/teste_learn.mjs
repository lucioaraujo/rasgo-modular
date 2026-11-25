import fs from 'fs';
const bytes = fs.readFileSync(new URL('./rasgo.wasm', import.meta.url));
let mem; const stub = new Proxy({}, { get: (_, n) => (...a) => { if (n === 'fd_write') { const v = new DataView(mem.buffer); let s = 0; for (let k = 0; k < a[2]; k++) s += v.getUint32(a[1]+8*k+4, true); v.setUint32(a[3], s, true); } return 0; } });
const { instance } = await WebAssembly.instantiate(bytes, { wasi_snapshot_preview1: stub, env: stub });
const e = instance.exports; mem = e.memory; e._initialize && e._initialize();
const txt = (t) => { const b = new TextEncoder().encode(t); new Uint8Array(mem.buffer, e.rw_entrada(b.length), b.length).set(b); return b.length; };
const c = (p) => { const u = new Uint8Array(mem.buffer); let f = p; while (u[f]) f++; return new TextDecoder().decode(u.slice(p, f)); };
let ok = true; const confere = (x, m) => { console.log((x ? 'ok  ' : 'FALHOU ') + m); ok &&= x; };
for (const l of [1, 0, 2, 3]) {
  const w = JSON.parse(c(e.rw_learn_widget(txt('FILTER\tcutoff'), l)) || 'null');
  const m = JSON.parse(c(e.rw_learn_modulo(txt('FILTER'), l)) || 'null');
  confere(w && m, `língua ${l}: FILTER.cutoff "${w?.q.slice(0, 50)}…" · módulo "${m?.q.slice(0, 40)}…"`);
}
confere(c(e.rw_learn_widget(txt('FILTER\tin:in'), 1)).length > 10, 'jack in:in');
confere(JSON.parse(c(e.rw_learn_cabo(1))).t.startsWith('CABO'), 'cabo');
confere(JSON.parse(c(e.rw_learn_cabecalho(txt('mutate'), 1))).t === 'MUDA', 'cabeçalho MUDA');
confere(c(e.rw_learn_ocioso(1)).startsWith('passe'), 'texto ocioso');
e.rw_iniciar(48000); e.rw_seed(1, 2);
const d = JSON.parse(new TextDecoder().decode(new Uint8Array(mem.buffer, e.rw_descrever(), e.rw_texto_tamanho())));
const f = d.modulos.find(x => x.tipo === 'FILTER');
console.log('valor:', JSON.stringify(c(e.rw_learn_valor(f.id, txt('cutoff')))));
// cobertura: quantos controles e jacks do rack têm verbete
let tem = 0, tot = 0;
for (const md of d.modulos) for (const w of md.w) if (w.b) { tot++; if (c(e.rw_learn_widget(txt(md.tipo + '\t' + w.b), 1))) tem++; }
console.log(`verbetes: ${tem}/${tot} widgets com bind`);
console.log(ok ? 'TUDO OK' : 'HÁ FALHAS');
