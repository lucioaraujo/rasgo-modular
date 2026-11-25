// O worklet num ambiente como o do AudioWorklet: sem TextDecoder/TextEncoder.
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
const seed = msgs.find((m) => m.tipo === 'seed');
let ok = true; const confere = (c, m) => { console.log((c ? 'ok  ' : 'FALHOU ') + m); ok &&= c; };
confere(!!seed, 'mensagem seed chegou');
const r = JSON.parse(seed.texto); confere(r.modulos.length === 105, `rack: ${r.modulos.length} módulos, ${seed.cabos} cabos, marca ${seed.wordmark.length} car.`);
p.mensagem({ tipo: 'visiveis', ids: r.modulos.slice(0, 20).map((m) => m.id) });
const L = new Float32Array(128), R = new Float32Array(128);
for (let i = 0; i < 400; i++) p.process([], [[L, R]]);
const q = msgs.filter((m) => m.tipo === 'quadro');
confere(q.length > 5 && q.at(-1).visores.length === 20 * 748, `${q.length} quadros, visores de 20 módulos`);
p.mensagem({ tipo: 'adicionar', modulo: 'OSC' });
confere(msgs.some((m) => m.tipo === 'rack' && m.novo > 0), 'acrescentar OSC devolve o rack');
p.mensagem({ tipo: 'salvar', destino: 'banco' });
confere(msgs.some((m) => m.tipo === 'salvo' && m.texto.length > 1000), 'salvar devolve o .rmp');
console.log(ok ? 'TUDO OK' : 'HÁ FALHAS');
