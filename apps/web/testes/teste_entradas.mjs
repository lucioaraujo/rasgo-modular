// Microfone e MIDI no SIGNAL-IN, pelo worklet (escopo como o do AudioWorklet).
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
const r = JSON.parse(msgs.find((m) => m.tipo === 'seed').texto);
const si = r.modulos.find((m) => m.tipo === 'SIGNAL-IN');
const on = si.w.find((w) => w.b === 'listen');
const L = new Float32Array(128), R = new Float32Array(128);
const quadro = () => { msgs.length = 0; for (let i = 0; i < 20; i++) p.process([[]], [[L, R]]); return msgs.filter((m) => m.tipo === 'quadro').at(-1); };
confere(quadro().entrada === 0, 'sem SIGNAL-IN ligado, a página não pede entrada');
p.mensagem({ tipo: 'alterna', i: on.v });
const mixer = r.modulos.find((m) => m.tipo === 'MIXER');
p.mensagem({ tipo: 'ligar', o: si.id, po: 0, d: mixer.id, pd: 3 });
confere(quadro().entrada === 3, `ON ligado e cabeado: pede microfone e MIDI (${quadro().entrada})`);
// um seno de 440 Hz entrando, e o visor do SIGNAL-IN mostrando sinal
p.mensagem({ tipo: 'visiveis', ids: [si.id] });
let fase = 0; const seno = new Float32Array(128);
msgs.length = 0;
for (let i = 0; i < 200; i++) {
  for (let k = 0; k < 128; k++) seno[k] = 0.5 * Math.sin((fase++) * 2 * Math.PI * 440 / 48000);
  p.process([[seno]], [[L, R]]);
}
const q = msgs.filter((m) => m.tipo === 'quadro').at(-1);
let pico = 0; for (let k = 0; k < 220; k++) pico = Math.max(pico, Math.abs(q.visores[k]));
confere(pico > 0.3, `o som do microfone chega à saída do SIGNAL-IN (pico ${pico.toFixed(2)})`);
p.mensagem({ tipo: 'midi', d: [0x90, 60, 100] }); quadro();
p.mensagem({ tipo: 'midi', d: [0x80, 60, 0] }); quadro();
confere(true, 'nota MIDI entregue sem erro');
// cortar TODAS as saídas do SIGNAL-IN (a seed pode ter ligado outras)
const ultimo = msgs.filter((m) => m.tipo === 'quadro').at(-1).cabos;
const portas = new Set();
for (let i = 0; i + 13 < ultimo.length; i += 14) if (ultimo[i] === si.id) portas.add(ultimo[i + 1]);
console.log('saídas do SIGNAL-IN cabeadas:', [...portas].join(', '));
for (const po of portas) p.mensagem({ tipo: 'cortarSaida', o: si.id, po });
confere(quadro().entrada === 0, 'cabos cortados: a entrada fecha');
console.log(ok ? 'TUDO OK' : 'HÁ FALHAS');
