import fs from 'fs';
const bytes = fs.readFileSync(new URL('./rasgo.wasm', import.meta.url));
let mem; const stub = new Proxy({}, { get: (_, n) => (...a) => { if (n === 'fd_write') { const v = new DataView(mem.buffer); let s = 0; for (let k = 0; k < a[2]; k++) s += v.getUint32(a[1]+8*k+4, true); v.setUint32(a[3], s, true); } return 0; } });
const { instance } = await WebAssembly.instantiate(bytes, { wasi_snapshot_preview1: stub, env: stub });
const e = instance.exports; mem = e.memory; e._initialize && e._initialize();
e.rw_iniciar(48000); const s = 4303935450909092226n; e.rw_seed(Number(s >> 32n), Number(s & 0xffffffffn));
const desc = () => JSON.parse(new TextDecoder().decode(new Uint8Array(mem.buffer, e.rw_descrever(), e.rw_texto_tamanho())));
const cabos = () => { const n = e.rw_cabos(); const c = new Float32Array(mem.buffer, e.rw_cabos_estado(), n * 14); const r = []; for (let i = 0; i < n; i++) r.push(Array.from(c.slice(i*14, i*14+14))); return r; };
const rms = (seg) => { let s2 = 0, n = 0; for (let b = 0; b < seg*375; b++) { const f = new Float32Array(mem.buffer, e.rw_processar(), 256); for (const x of f) { s2 += x*x; n++; } } return (10*Math.log10(s2/n + 1e-20)).toFixed(1); };
let ok = true; const confere = (c, msg) => { console.log((c ? 'ok  ' : 'FALHOU ') + msg); ok &&= c; };
let r = desc();
console.log('nível inicial', rms(2), 'dBFS');
// 1. controle: o FREQ do OSC
const osc = r.modulos.find(m => m.tipo === 'OSC'); const fq = osc.w.find(w => w.b === 'freq');
e.rw_mao(1); e.rw_param(fq.v, 440); rms(0.2);
confere(Math.abs(e.rw_param_usuario(fq.v) - 440) < 0.01, `rw_param: FREQ do OSC = ${e.rw_param_usuario(fq.v).toFixed(2)} (pedido 440)`);
e.rw_param(fq.v, 99999); confere(e.rw_param_usuario(fq.v) <= fq.max, `valor acima da faixa fica no teto (${e.rw_param_usuario(fq.v)})`);
e.rw_mao(0);
// 2. cortar e religar o cabo que chega ao MASTER
const n0 = e.rw_cabos(); const master = r.modulos.find(m => m.tipo === 'MASTER');
const entra = cabos().find(c => c[2] === master.id);
confere(!!entra, `há cabo chegando ao MASTER (${entra})`);
confere(e.rw_cortar_entrada(entra[2], entra[3]) === 1 && e.rw_cabos() === n0 - 1, `cortar a entrada: ${n0} → ${e.rw_cabos()} cabos`);
r = desc(); confere(r.modulos.filter(m => m.chega).length < 34, `sem esse cabo, menos módulos chegam à saída (${r.modulos.filter(m => m.chega).length})`);
console.log('nível sem o cabo do MASTER', rms(1), 'dBFS');
confere(e.rw_ligar(entra[0], entra[1], entra[2], entra[3]) === 1 && e.rw_cabos() === n0, `religar: ${e.rw_cabos()} cabos`);
r = desc(); confere(r.modulos.filter(m => m.chega).length === 34, `de volta: ${r.modulos.filter(m => m.chega).length} chegam à saída`);
console.log('nível religado', rms(2), 'dBFS');
// 3. um laço: a saída do MIXER de volta numa entrada que o alimenta
const mixer = r.modulos.find(m => m.tipo === 'MIXER'); const entraMix = cabos().find(c => c[2] === mixer.id);
const antes = e.rw_cabos();
e.rw_ligar(mixer.id, 0, entraMix[0], 0);
const novo = cabos().find(c => c[0] === mixer.id && c[2] === entraMix[0]);
confere(!!novo, `laço aceito como realimentação (MIXER → nó ${entraMix[0]}): ${antes} → ${e.rw_cabos()} cabos`);
const nivel = rms(2); confere(Number(nivel) < 0 && isFinite(Number(nivel)), `com o laço o som segue finito (${nivel} dBFS)`);
// 4. cortar todas as saídas de um jack
const fonte = cabos()[0]; const m0 = e.rw_cabos();
e.rw_cortar_saida(fonte[0], fonte[1]);
confere(!cabos().some(c => c[0] === fonte[0] && c[1] === fonte[1] && c[2] !== r.saida), `cortar a saída ${fonte[0]}:${fonte[1]}: ${m0} → ${e.rw_cabos()} cabos`);
console.log(ok ? 'TUDO OK' : 'HÁ FALHAS');
// 5. o inspetor: no cabo que chega ao MASTER
const alvo = cabos().find(c => c[2] === master.id);
const achar = () => cabos().find(c => c[2] === alvo[2] && c[3] === alvo[3]);
e.rw_cabo_ganho(alvo[2], alvo[3], 0.5); confere(Math.abs(achar()[6] - 0.5) < 1e-6, `GAIN 0,5 (${achar()[6]})`);
const nv = rms(1); console.log('nível com GAIN 0,5 no cabo do MASTER', nv, 'dBFS');
e.rw_cabo_ganho(alvo[2], alvo[3], 1);
e.rw_cabo_condutancia(alvo[2], alvo[3], 0.3); confere(Math.abs(achar()[8] - 0.3) < 1e-6, `COND 0,3 (${achar()[8].toFixed(2)})`);
let pisca = 0; for (let b = 0; b < 200; b++) { e.rw_processar(); if (!achar()[5]) pisca++; }
confere(pisca > 0 && pisca < 200, `com COND 0,3 o cabo às vezes não conduz (${pisca}/200 blocos)`);
e.rw_cabo_condutancia(alvo[2], alvo[3], 1);
e.rw_cabo_relacao(alvo[2], alvo[3], 1, fonte[0], fonte[1], 0.7);
let a = achar(); confere(a[7] === 1 && a[9] === 1 && Math.abs(a[10] - 0.7) < 1e-6 && a[11] === fonte[0], `RING com companion ${a[11]}:${a[12]}, AMT ${a[10].toFixed(2)}`);
const nr = rms(1); confere(isFinite(Number(nr)), `com a relação o som segue (${nr} dBFS)`);
e.rw_cabo_relacao(alvo[2], alvo[3], 0, 0, 0, 0); confere(achar()[7] === 0, 'NONE tira a relação');
e.rw_cabo_romper(alvo[2], alvo[3]); confere(achar()[4] === 1, 'ROMPER');
console.log('nível rompido', rms(1), 'dBFS');
e.rw_cabo_romper(alvo[2], alvo[3]); confere(achar()[4] === 0, 'RECONECTAR');
console.log(ok ? 'TUDO OK' : 'HÁ FALHAS');
