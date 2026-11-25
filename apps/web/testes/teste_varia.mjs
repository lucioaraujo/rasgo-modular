import fs from 'fs';
const TAXA = Number(process.env.TAXA || 48000);
const bytes = fs.readFileSync(new URL('./rasgo.wasm', import.meta.url));
async function rodar(seed, varia, seg) {
  let mem;
  const stub = new Proxy({}, { get: (_, n) => (...a) => {
    if (n === 'fd_write') { const v = new DataView(mem.buffer); let s = 0; for (let k = 0; k < a[2]; k++) s += v.getUint32(a[1] + 8*k + 4, true); v.setUint32(a[3], s, true); }
    return 0; } });
  const { instance } = await WebAssembly.instantiate(bytes, { wasi_snapshot_preview1: stub, env: stub });
  const e = instance.exports; mem = e.memory; if (e._initialize) e._initialize();
  e.rw_iniciar(TAXA); e.rw_varia(varia ? 1 : 0, 1.0);
  e.rw_seed(Number(seed >> 32n), Number(seed & 0xffffffffn));
  const blocos = Math.round(seg * TAXA / 128), porSeg = Math.round(TAXA/128); let pico = 0, soma = 0, n = 0, segs = [], s1 = 0, zc = 0, zcs = [], ant = 0;
  const t0 = Date.now();
  for (let b = 0; b < blocos; b++) {
    const f = new Float32Array(mem.buffer, e.rw_processar(), 256);
    for (let i = 0; i < 256; i += 2) { const x = f[i]; pico = Math.max(pico, Math.abs(x), Math.abs(f[i+1])); soma += x*x; s1 += x*x; n++; if ((x >= 0) !== (ant >= 0)) zc++; ant = x; }
    if ((b + 1) % porSeg === 0) { segs.push(10*Math.log10(s1/(porSeg*128)+1e-12)); zcs.push(zc); s1 = 0; zc = 0; }
  }
  const ms = Date.now() - t0;
  return { ms, x: (seg*1000/ms).toFixed(1), rms: (10*Math.log10(soma/n)).toFixed(1), pico: (20*Math.log10(pico)).toFixed(1), segs, zcs };
}
const seeds = process.argv.slice(2).map(BigInt);
for (const s of seeds) {
  const off = await rodar(s, false, 30), on = await rodar(s, true, 30);
  const dif = on.segs.map((v, i) => Math.abs(v - off.segs[i])), dzc = on.zcs.map((v, i) => Math.abs(v - off.zcs[i]) / Math.max(1, off.zcs[i]));
  console.log(`seed ${s}\n  sem VARIA: ${off.x}x tempo real, RMS ${off.rms}, pico ${off.pico}\n  com VARIA: ${on.x}x tempo real, RMS ${on.rms}, pico ${on.pico}`);
  console.log(`  diferença por segundo (dB): ${dif.map(d => d.toFixed(1)).join(' ')}`);
  console.log(`  diferença de brilho (cruzamentos de zero, %): ${dzc.map(d => (100*d).toFixed(0)).join(' ')}`);
}
