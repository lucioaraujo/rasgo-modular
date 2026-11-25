import fs from 'fs';
const bytes = fs.readFileSync(new URL('./rasgo.wasm', import.meta.url));
let mem; const stub = new Proxy({}, { get: (_, n) => (...a) => { if (n === 'fd_write') { const v = new DataView(mem.buffer); let s = 0; for (let k = 0; k < a[2]; k++) s += v.getUint32(a[1]+8*k+4, true); v.setUint32(a[3], s, true); } return 0; } });
const { instance } = await WebAssembly.instantiate(bytes, { wasi_snapshot_preview1: stub, env: stub });
const e = instance.exports; mem = e.memory; e._initialize && e._initialize();
e.rw_iniciar(48000);
const S = 4303935450909092226n; e.rw_seed(Number(S >> 32n), Number(S & 0xffffffffn));
const desc = () => JSON.parse(new TextDecoder().decode(new Uint8Array(mem.buffer, e.rw_descrever(), e.rw_texto_tamanho())));
const txt = (t) => { const b = new TextEncoder().encode(t); new Uint8Array(mem.buffer, e.rw_entrada(b.length), b.length).set(b); return b.length; };
const rms = (seg) => { let s2 = 0, n = 0; for (let b = 0; b < seg*375; b++) { const f = new Float32Array(mem.buffer, e.rw_processar(), 256); for (const x of f) { s2 += x*x; n++; } } return 10*Math.log10(s2/n + 1e-20); };
let ok = true; const confere = (c, m) => { console.log((c ? 'ok  ' : 'FALHOU ') + m); ok &&= c; };
let d = desc(); const n0 = d.modulos.length;
// acrescentar um OSC: entra junto da família SOURCE, com número de instância
const id = e.rw_adicionar(txt('OSC')); d = desc();
const k = d.modulos.findIndex(m => m.id === id);
confere(id > 0 && d.modulos.length === n0 + 1, `+OSC: id ${id}, ${d.modulos.length} módulos`);
confere(d.modulos[k].familia === 'SOURCE' && d.modulos[k].inst > 1, `entrou na SOURCE (posição ${k}), instância ${d.modulos[k].inst}`);
confere(e.rw_adicionar(txt('MASTER')) === -1, 'MASTER é único: recusado');
let r; for (let i = 0; i < 6; i++) r = e.rw_adicionar(txt('FILTER')); confere(r === -2, 'FILTER: no máximo 4 cópias');
confere(e.rw_adicionar(txt('NAOEXISTE')) === -3, 'tipo inexistente recusado');
// tirar
const osc0 = desc().modulos.find(m => m.tipo === 'OSC');
const antes = e.rw_cabos();
confere(e.rw_remover(osc0.id) === 1 && !desc().modulos.some(m => m.id === osc0.id), `tirar o OSC ${osc0.id}: ${antes} → ${e.rw_cabos()} cabos`);
confere(isFinite(rms(1)), 'segue soando');
e.rw_desfazer(); confere(desc().modulos.some(m => m.id === osc0.id) && e.rw_cabos() === antes, 'DESFAZ devolve o módulo e os cabos');
// reordenar: o último vai para o começo
d = desc(); const ids = d.modulos.map(m => m.id); const ult = ids.pop(); ids.unshift(ult);
e.rw_ordem(txt(ids.join(' '))); confere(desc().modulos[0].id === ult, `reordenar: ${desc().modulos[0].tipo} no começo`);
// salvar e abrir guarda a ordem
const sv = new TextDecoder().decode(new Uint8Array(mem.buffer, e.rw_salvar(), e.rw_salvo_tamanho()).slice());
e.rw_seed(1, 2); const b = new TextEncoder().encode(sv); new Uint8Array(mem.buffer, e.rw_entrada(b.length), b.length).set(b); e.rw_abrir(b.length);
confere(desc().modulos[0].id === ult, 'a ordem volta com o .rmp');
// banco de escalas
const p = e.rw_banco(); let f = p; const u8 = new Uint8Array(mem.buffer); while (u8[f]) f++;
const banco = JSON.parse(new TextDecoder().decode(u8.slice(p, f)));
confere(banco.escalas.length === 2576, `banco: ${banco.escalas.length} escalas, ${banco.familias.length} famílias, ${((f-p)/1024).toFixed(0)} KB`);
const q = desc().modulos.find(m => m.tipo === 'QUANTIZER');
const esc = banco.escalas[100]; e.rw_escolher_escala(q.id, esc.id);
const pn = e.rw_nome_escala(q.id); let pf = pn; while (u8[pf]) pf++;
confere(new TextDecoder().decode(new Uint8Array(mem.buffer).slice(pn, pf)) === esc.n, `QUANTIZER toca "${esc.n}"`);
confere(Math.round(e.rw_parametro(q.id, txt('bank'))) === esc.id, 'rw_parametro lê o bank');
console.log(ok ? 'TUDO OK' : 'HÁ FALHAS');
