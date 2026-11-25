import fs from 'fs';
const bytes = fs.readFileSync(new URL('./rasgo.wasm', import.meta.url));
let mem; const stub = new Proxy({}, { get: (_, n) => (...a) => { if (n === 'fd_write') { const v = new DataView(mem.buffer); let s = 0; for (let k = 0; k < a[2]; k++) s += v.getUint32(a[1]+8*k+4, true); v.setUint32(a[3], s, true); } return 0; } });
const { instance } = await WebAssembly.instantiate(bytes, { wasi_snapshot_preview1: stub, env: stub });
const e = instance.exports; mem = e.memory; e._initialize && e._initialize();
e.rw_iniciar(48000); const s = 4303935450909092226n; e.rw_seed(Number(s >> 32n), Number(s & 0xffffffffn));
const r = JSON.parse(new TextDecoder().decode(new Uint8Array(mem.buffer, e.rw_descrever(), e.rw_texto_tamanho())));
const t0 = performance.now();
for (let b = 0; b < 3*375; b++) e.rw_processar();
console.log('3 s de áudio em', (performance.now()-t0).toFixed(0), 'ms');
const n = e.rw_num_visores(); console.log('visores', n);
const tipos = new Map(r.modulos.map(m => [m.id, m.tipo]));
for (let k = 0; k < n; k++) {
  const id = e.rw_visor_id(k), v = new Float32Array(mem.buffer, e.rw_visor(id), 748);
  let pk = 0; for (let i = 0; i < 220; i++) pk = Math.max(pk, Math.abs(v[i]));
  let ln = 0; for (let i = 220; i < 732; i++) ln = Math.max(ln, Math.abs(v[i]));
  const ex = Array.from(v.slice(732, 748)).map(x => +x.toFixed(2)).filter((x,i,a)=>a.slice(i).some(y=>y));
  console.log(String(id).padStart(3), tipos.get(id).padEnd(10), 'pico', pk.toFixed(3), 'lanes', ln.toFixed(2), ex.length ? 'extras ' + ex.join(' ') : '');
}
const q = r.modulos.find(m => m.tipo === 'QUANTIZER');
const p = e.rw_nome_escala(q.id), b = new Uint8Array(mem.buffer); let f = p; while (b[f]) f++;
console.log('QUANTIZER escala do banco:', new TextDecoder().decode(b.slice(p, f)));
