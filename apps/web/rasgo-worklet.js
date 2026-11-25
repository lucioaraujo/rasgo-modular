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
    this.nc = 0;
    this.quadro = 0;   // blocos desde o último quadro mandado à página
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
      // o rack para a página desenhar (uma vez por seed)
      const dec = new TextDecoder();
      const texto = dec.decode(new Uint8Array(this.mem.buffer, this.e.rw_descrever(), this.e.rw_texto_tamanho()).slice());
      const wp = this.e.rw_wordmark(), b = new Uint8Array(this.mem.buffer);
      let fim = wp; while (b[fim]) fim++;
      const wordmark = dec.decode(b.slice(wp, fim));
      this.nc = this.e.rw_num_controles();
      this.port.postMessage({ tipo: 'seed', seed: m.seed, cabos: this.e.rw_cabos(), texto, wordmark });
    } else if (m.tipo === 'parar') {
      this.tocando = false;
    } else if (m.tipo === 'varia' && this.e) {
      this.e.rw_varia(m.ligado ? 1 : 0, m.intensidade);
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
    // ~30 quadros por segundo: os valores dos controles e os cabos
    if (++this.quadro >= Math.round(sampleRate / 30 / 128)) {
      this.quadro = 0;
      const v = new Float32Array(this.mem.buffer, this.e.rw_valores(), this.nc).slice();
      const nCab = this.e.rw_cabos();
      const c = new Float32Array(this.mem.buffer, this.e.rw_cabos_estado(), nCab * 9).slice();
      this.port.postMessage({ tipo: 'quadro', valores: v, cabos: c }, [v.buffer, c.buffer]);
    }
    return true;
  }
}

registerProcessor('rasgo', RasgoProcessor);
