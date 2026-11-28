// Monta os arquivos da tomada (REC) FORA da página: o WAV de 24 bits, a
// versão de entrega a -14 LUFS e o texto das medidas (rasgo_rec.cpp). Antes
// isso rodava na página e, por alguns segundos depois de parar a gravação,
// as animações do rack travavam — o som seguia, porque mora no worklet
// (autor, 27/11/2026). Aqui o trabalho pesado tem a sua própria instância do
// motor e a página só recebe os bytes prontos.
let e = null;

async function instanciar(bytes) {
  let mem;
  const stub = new Proxy({}, { get: (_, n) => (...a) => {
    if (n === 'fd_write') { const v = new DataView(mem.buffer); let s = 0;
      for (let k = 0; k < a[2]; k++) s += v.getUint32(a[1] + 8 * k + 4, true); v.setUint32(a[3], s, true); }
    return 0; } });
  const { instance } = await WebAssembly.instantiate(bytes, { wasi_snapshot_preview1: stub, env: stub });
  const x = instance.exports; mem = x.memory; if (x._initialize) x._initialize();
  return x;
}

function texto(p) {
  const u8 = new Uint8Array(e.memory.buffer); let f = p; while (u8[f]) f++;
  return new TextDecoder().decode(u8.slice(p, f));
}

onmessage = async (ev) => {
  const { bytes, blocos, taxa, nome } = ev.data;
  try {
    if (!e) e = await instanciar(bytes);
    const total = blocos.reduce((n, b) => n + b.length, 0);
    const p = e.rw_rec_buffer(total);
    let off = 0;
    for (const b of blocos) { new Float32Array(e.memory.buffer, p, total).set(b, off); off += b.length; }
    const arquivo = () => new Uint8Array(e.memory.buffer, e.rw_rec_arquivo(), e.rw_rec_bytes()).slice();
    e.rw_rec_wav(taxa);
    const wav = arquivo();
    let entrega = null, medidas = '';
    const nb = new TextEncoder().encode(nome + '.wav');
    const np = e.rw_entrada(nb.length); new Uint8Array(e.memory.buffer, np, nb.length).set(nb);
    if (e.rw_rec_entrega(taxa, np)) { entrega = arquivo(); medidas = texto(e.rw_rec_texto()); }
    e.rw_rec_liberar();
    const trans = [wav.buffer]; if (entrega) trans.push(entrega.buffer);
    postMessage({ wav, entrega, medidas }, trans);
  } catch (x) {
    postMessage({ erro: String(x) });
  }
};
