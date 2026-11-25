import fs from 'fs';
const bytes = fs.readFileSync(new URL('./rasgo.wasm', import.meta.url));
let mem; const stub = new Proxy({}, { get: (_, n) => (...a) => { if (n === 'fd_write') { const v = new DataView(mem.buffer); let s = 0; for (let k = 0; k < a[2]; k++) s += v.getUint32(a[1]+8*k+4, true); v.setUint32(a[3], s, true); } return 0; } });
const { instance } = await WebAssembly.instantiate(bytes, { wasi_snapshot_preview1: stub, env: stub });
const e = instance.exports; mem = e.memory; e._initialize && e._initialize();
e.rw_iniciar(48000);
const S = 4303935450909092226n, hi = (s) => Number(s >> 32n), lo = (s) => Number(s & 0xffffffffn);
const seed = () => (BigInt(e.rw_seed_alto()) << 32n) | BigInt(e.rw_seed_baixo());
const desc = () => JSON.parse(new TextDecoder().decode(new Uint8Array(mem.buffer, e.rw_descrever(), e.rw_texto_tamanho())));
const rms = (seg) => { let s2 = 0, n = 0; for (let b = 0; b < seg*375; b++) { const f = new Float32Array(mem.buffer, e.rw_processar(), 256); for (const x of f) { s2 += x*x; n++; } } return 10*Math.log10(s2/n + 1e-20); };
const salvar = () => new TextDecoder().decode(new Uint8Array(mem.buffer, e.rw_salvar(), e.rw_salvo_tamanho()).slice());
const abrir = (t) => { const b = new TextEncoder().encode(t); new Uint8Array(mem.buffer, e.rw_entrada(b.length), b.length).set(b); return e.rw_abrir(b.length); };
let ok = true; const confere = (c, m) => { console.log((c ? 'ok  ' : 'FALHOU ') + m); ok &&= c; };
e.rw_seed(hi(S), lo(S));
confere(seed() === S && !e.rw_pode_desfazer(), `seed ${seed()}, pilha vazia`);
const n0 = e.rw_cabos(), t0 = salvar(), r0 = rms(2);
console.log('nível', r0.toFixed(1), 'dBFS ·', n0, 'cabos · .rmp de', t0.length, 'bytes');
// DESCABEIA e DESFAZ
e.rw_descabear(); confere(e.rw_cabos() === 1, `DESCABEIA: ${e.rw_cabos()} cabo (o da saída)`);
const rm = rms(1); confere(rm < -60, `sem cabos o som some (${rm.toFixed(1)} dBFS)`);
confere(e.rw_desfazer() === 1 && e.rw_cabos() === n0 && seed() === S, `DESFAZ: ${e.rw_cabos()} cabos, seed ${seed()}`);
const rd = rms(2); confere(Math.abs(rd - r0) < 6, `desfeito, o som volta (${rd.toFixed(1)} dBFS)`);
// MUDA muda parâmetros, DESFAZ volta
const a = salvar(); e.rw_mudar(123, 456); const b = salvar();
confere(a !== b, 'MUDA mexe em parâmetros'); e.rw_desfazer(); confere(salvar() === a, 'DESFAZ volta ao mesmo texto');
e.rw_evoluir(7, 8); confere(salvar() !== a, 'EVOLUI'); const re = rms(1); confere(isFinite(re), `som após EVOLUI ${re.toFixed(1)} dBFS`);
e.rw_cruzar(9, 10, 11, 12); const rc = rms(1); confere(isFinite(rc), `CRUZA, som ${rc.toFixed(1)} dBFS`);
// MIXER e MASTER congelados: os valores do MASTER não mudam
const mst = (t) => t.split('\n').filter(l => l.startsWith('param ') && l.includes(String(desc().modulos.find(m => m.tipo === 'MASTER').id) + ' ')).join('|');
// REPOR
confere(e.rw_repor() === 1 && e.rw_cabos() === n0, `REPOR: ${e.rw_cabos()} cabos`);
// ESPERA
e.rw_espera(1); rms(0.5); const rs = rms(1); confere(rs < r0 - 30, `ESPERA cala (${rs.toFixed(1)} dBFS)`); e.rw_espera(0); rms(0.5);
// SALVA → ABRIR (outro patch no meio)
e.rw_seed(1, 2); const nOutro = e.rw_cabos();
confere(abrir(t0) === 1 && e.rw_cabos() === n0 && seed() === S, `ABRIR o .rmp: ${nOutro} → ${e.rw_cabos()} cabos, seed ${seed()}`);
const d = desc(); confere(d.modulos.length === 105 && d.modulos.filter(m => m.chega).length === 34, `${d.modulos.length} módulos, ${d.modulos.filter(m => m.chega).length} chegam à saída`);
const ra = rms(2); confere(Math.abs(ra - r0) < 6, `o patch aberto soa (${ra.toFixed(1)} dBFS, antes ${r0.toFixed(1)})`);
confere(abrir('lixo qualquer') === 0 && e.rw_cabos() === n0, 'arquivo inválido: recusado, o patch fica');
console.log('pilha do DESFAZ:', e.rw_pode_desfazer() ? 'com fotos' : 'vazia');
let k = 0; while (e.rw_desfazer()) k++; console.log('desfeitos', k, '(limite 24)');
console.log(ok ? 'TUDO OK' : 'HÁ FALHAS');
