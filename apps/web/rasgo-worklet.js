// O motor do Rasgo Modular (rasgo.wasm) dentro de um AudioWorklet. A página
// manda os bytes do .wasm e as seeds; aqui cada quantum de 128 amostras sai
// do mesmo motor do app (24/11/2026).

class RasgoProcessor extends AudioWorkletProcessor {
  constructor() {
    super();
    this.e = null;
    this.mem = null;
    this.tocando = false;
    this.ganho = 1.0;
    this.port.onmessage = (ev) => this.mensagem(ev.data);
  }

  async mensagem(m) {
    if (m.tipo === 'wasm') {
      const self = this;
      const stub = new Proxy({}, { get: (_, nome) => (...a) => {
        if (nome === 'fd_write' && self.mem) {
          const v = new DataView(self.mem.buffer); let s = 0;
          for (let k = 0; k < a[2]; k++) s += v.getUint32(a[1] + 8 * k + 4, true);
          v.setUint32(a[3], s, true);
        }
        return 0;
      } });
      const { instance } = await WebAssembly.instantiate(m.bytes, { wasi_snapshot_preview1: stub, env: stub });
      this.e = instance.exports;
      this.mem = this.e.memory;
      if (this.e._initialize) this.e._initialize();
      this.e.rw_iniciar(sampleRate);
      this.port.postMessage({ tipo: 'pronto' });
    } else if (m.tipo === 'seed' && this.e) {
      const s = BigInt(m.seed);
      this.e.rw_seed(Number(s >> 32n), Number(s & 0xffffffffn));
      this.tocando = true;
      this.port.postMessage({ tipo: 'seed', seed: m.seed, cabos: this.e.rw_cabos() });
    } else if (m.tipo === 'parar') {
      this.tocando = false;
    } else if (m.tipo === 'ganho') {
      this.ganho = m.valor;
    }
  }

  process(_in, outputs) {
    const out = outputs[0];
    if (!this.e || !this.tocando || out.length < 2) return true;
    const p = this.e.rw_processar();
    const f = new Float32Array(this.mem.buffer, p, 256);
    const L = out[0], R = out[1], g = this.ganho;
    for (let i = 0; i < L.length; i++) { L[i] = f[2 * i] * g; R[i] = f[2 * i + 1] * g; }
    return true;
  }
}

registerProcessor('rasgo', RasgoProcessor);
