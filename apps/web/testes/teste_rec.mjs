// O REC: grava ~3 s pelo worklet (escopo sem TextDecoder, como o AudioWorklet),
// e monta o WAV e a versão de entrega no motor da página.
import fs from 'fs'; import vm from 'vm';
const src = fs.readFileSync(new URL('./rasgo-worklet.js', import.meta.url), 'utf8');
const bytes = fs.readFileSync(new URL('./rasgo.wasm', import.meta.url));
let Classe = null; const msgs = [];
const ctx = { WebAssembly, Proxy, BigInt, Uint8Array, Float32Array, DataView, Math, JSON, Object, String, Number, Array, Boolean, Date,
  sampleRate: 48000, console,
  AudioWorkletProcessor: class { constructor() { this.port = { postMessage: (m) => msgs.push(m), onmessage: null }; } },
  registerProcessor: (_, c) => { Classe = c; } };
vm.createContext(ctx); vm.runInContext(src, ctx);
const p = new Classe();
await p.mensagem({ tipo: 'wasm', bytes });
await p.mensagem({ tipo: 'seed', seed: '4303935450909092226' });
let ok = true; const confere = (c, m) => { console.log((c ? 'ok  ' : 'FALHOU ') + m); ok &&= c; };
const L = new Float32Array(128), R = new Float32Array(128);
for (let i = 0; i < 100; i++) p.process([], [[L, R]]);
p.mensagem({ tipo: 'rec', ligar: true });
for (let i = 0; i < 3 * 375; i++) {
  p.process([], [[L, R]]);
  if (i === 400) { p.mensagem({ tipo: 'pega', i: 3 }); p.mensagem({ tipo: 'arrasto', i: 3, dy: 0.2, lo: 0, hi: 1 }); p.mensagem({ tipo: 'solta' }); }
}
const q = msgs.filter((m) => m.tipo === 'quadro').at(-1);
confere(q.rec > 2.9 && q.rec < 3.1, `relógio do REC no quadro: ${q.rec.toFixed(2)} s`);
p.mensagem({ tipo: 'rec', ligar: false });
const blocos = msgs.filter((m) => m.tipo === 'recAudio'), fim = msgs.find((m) => m.tipo === 'recFim');
const total = blocos.reduce((n, m) => n + m.bloco.length, 0);
confere(!!fim && total === fim.quadros * 2, `áudio: ${blocos.length} blocos, ${total} números = ${fim.quadros} quadros × 2`);
confere(/rec-\d{8}-\d{6}-seed4303935450909092226$/.test(fim.nome), `nome: ${fim.nome}`);
const linhas = fim.partitura.split('\n');
console.log(linhas.slice(0, 9).join('\n'));
confere(fim.partitura.includes('(navegador)') && fim.partitura.includes('24 bits, 48000 Hz, 3.0 s'), 'cabeçalho da partitura');
confere(/varia ligado/.test(fim.partitura), 'o estado do VARIA em t = 0');
confere(fim.partitura.split('\n').length > 70, `partitura com ${fim.partitura.split('\n').length} linhas`);
// os arquivos, no motor da página
let mem; const stub = new Proxy({}, { get: (_, n) => (...a) => { if (n === 'fd_write') { const v = new DataView(mem.buffer); let s = 0; for (let k = 0; k < a[2]; k++) s += v.getUint32(a[1]+8*k+4, true); v.setUint32(a[3], s, true); } return 0; } });
const { instance } = await WebAssembly.instantiate(bytes, { wasi_snapshot_preview1: stub, env: stub });
const e = instance.exports; mem = e.memory; e._initialize && e._initialize();
const ptr = e.rw_rec_buffer(total); let off = 0;
for (const b of blocos) { new Float32Array(mem.buffer, ptr, total).set(b.bloco, off); off += b.bloco.length; }
e.rw_rec_wav(48000);
const wav = new Uint8Array(mem.buffer, e.rw_rec_arquivo(), e.rw_rec_bytes()).slice();
const dv = new DataView(wav.buffer);
confere(new TextDecoder().decode(wav.slice(0, 4)) === 'RIFF' && dv.getUint16(34, true) === 24 && dv.getUint32(24, true) === 48000
        && wav.length === 44 + fim.quadros * 6, `WAV 24 bits: ${wav.length} bytes`);
const nb = new TextEncoder().encode(fim.nome + '.wav'), np = e.rw_entrada(nb.length);
new Uint8Array(mem.buffer, np, nb.length).set(nb);
const t0 = performance.now(), entrega = e.rw_rec_entrega(48000, np);
const u = new Uint8Array(mem.buffer); let f = e.rw_rec_texto(); const ti = f; while (u[f]) f++;
console.log(new TextDecoder().decode(u.slice(ti, f)).trim());
confere(entrega === 1 && e.rw_rec_bytes() === 44 + fim.quadros * 6, `versão de entrega em ${(performance.now() - t0).toFixed(0)} ms`);
e.rw_rec_liberar();
console.log(ok ? 'TUDO OK' : 'HÁ FALHAS');
