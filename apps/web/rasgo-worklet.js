// O motor do Rasgo Modular (rasgo.wasm) dentro de um AudioWorklet. A página
// manda os bytes do .wasm e as seeds; aqui cada quantum de 128 amostras sai
// do mesmo motor do app (24/11/2026).

// O arrasto de knob/slider do app (`RackView::mouseDrag`): 220 px de
// movimento vertical = a faixa inteira; faixas largas (hi/lo > 30, as
// frequências) andam em escala exponencial.
function valorArrastado(v0, dy, lo, hi) {
  let v;
  if (lo > 0 && hi / lo > 30) {
    const k = Math.log(hi / lo);
    const t = Math.min(1, Math.max(0, Math.log(Math.max(v0, lo) / lo) / k + dy));
    v = lo * Math.exp(k * t);
  } else {
    v = v0 + dy * (hi - lo);
  }
  return Math.min(hi, Math.max(lo, v));
}

// A roda do mouse num controle (`wheelOnControl`): ~2 % da faixa por clique
// (Shift: ~0,5 %); seletores de 2 a 16 valores andam um passo.
function valorRoda(v0, dir, fino, lo, hi) {
  const discreto = hi - lo >= 2 && hi - lo <= 16 && Math.floor(lo) === lo && Math.floor(hi) === hi;
  const passo = fino ? 0.005 : 0.02;
  let v;
  if (discreto) v = Math.round(v0) + dir;
  else if (lo > 0 && hi / lo > 30) {
    const k = Math.log(hi / lo);
    const t = Math.min(1, Math.max(0, Math.log(Math.max(v0, lo) / lo) / k + dir * passo));
    v = lo * Math.exp(k * t);
  } else v = v0 + dir * passo * (hi - lo);
  return Math.min(hi, Math.max(lo, v));
}

class RasgoProcessor extends AudioWorkletProcessor {
  constructor() {
    super();
    this.e = null;
    this.mem = null;
    this.tocando = false;
    this.ganho = 1.0;
    this.nc = 0;
    this.quadro = 0;   // blocos desde o último quadro mandado à página
    this.visiveis = [];   // ids dos visores na tela (a página diz quais)
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
      this.mandarRack('seed');
    } else if (m.tipo === 'parar') {
      this.tocando = false;
    } else if (m.tipo === 'pega' && this.e) {
      // o músico pegou um knob/slider: o arrasto parte do valor DELE (sem
      // a modulação), e a mão do VARIA espera até ele soltar
      this.arrasto = { i: m.i, v0: this.e.rw_param_usuario(m.i) };
      this.e.rw_mao(1);
    } else if (m.tipo === 'arrasto' && this.e && this.arrasto) {
      this.e.rw_param(m.i, valorArrastado(this.arrasto.v0, m.dy, m.lo, m.hi));
    } else if (m.tipo === 'solta' && this.e) {
      this.arrasto = null;
      this.e.rw_mao(0);
    } else if (m.tipo === 'mao' && this.e) {
      this.e.rw_mao(m.ativa ? 1 : 0);
    } else if (m.tipo === 'alterna' && this.e) {
      this.e.rw_param(m.i, this.e.rw_param_usuario(m.i) >= 0.5 ? 0 : 1);
    } else if (m.tipo === 'roda' && this.e) {
      this.e.rw_param(m.i, valorRoda(this.e.rw_param_usuario(m.i), m.dir, m.fino, m.lo, m.hi));
    } else if ((m.tipo === 'ligar' || m.tipo === 'cortarEntrada' || m.tipo === 'cortarSaida') && this.e) {
      const mudou = m.tipo === 'ligar' ? this.e.rw_ligar(m.o, m.po, m.d, m.pd)
        : m.tipo === 'cortarEntrada' ? this.e.rw_cortar_entrada(m.d, m.pd)
        : this.e.rw_cortar_saida(m.o, m.po);
      if (mudou) this.mandarRack('rack');
    } else if (m.tipo === 'caboRelacao' && this.e) {
      this.e.rw_cabo_relacao(m.d, m.pd, m.rel, m.cn ?? 0, m.cp ?? 0, m.amt ?? 0);
    } else if (m.tipo === 'caboCondutancia' && this.e) {
      this.e.rw_cabo_condutancia(m.d, m.pd, m.v);
    } else if (m.tipo === 'caboGanho' && this.e) {
      this.e.rw_cabo_ganho(m.d, m.pd, m.v);
    } else if (m.tipo === 'caboRomper' && this.e) {
      this.e.rw_cabo_romper(m.d, m.pd);
    } else if (m.tipo === 'varia' && this.e) {
      this.e.rw_varia(m.ligado ? 1 : 0, m.intensidade);
    } else if (m.tipo === 'foto' && this.e) {
      this.e.rw_foto();
    } else if (m.tipo === 'desfazer' && this.e) {
      if (this.e.rw_desfazer()) this.mandarRack('patch');
      else this.port.postMessage({ tipo: 'nada', acao: 'desfazer' });
    } else if (m.tipo === 'repor' && this.e) {
      if (this.e.rw_repor()) this.mandarRack('patch');
    } else if (m.tipo === 'mudar' && this.e) {
      this.e.rw_mudar(m.s[0], m.s[1]);
    } else if (m.tipo === 'evoluir' && this.e) {
      this.e.rw_evoluir(m.s[0], m.s[1]);
    } else if (m.tipo === 'cruzar' && this.e) {
      this.e.rw_cruzar(m.s[0], m.s[1], m.s[2], m.s[3]);
    } else if (m.tipo === 'descabear' && this.e) {
      this.e.rw_descabear();
      this.mandarRack('rack');
    } else if (m.tipo === 'romperTodos' && this.e) {
      this.e.rw_romper_todos();
    } else if (m.tipo === 'espera' && this.e) {
      this.e.rw_espera(m.ligada ? 1 : 0);
    } else if (m.tipo === 'salvar' && this.e) {
      const p = this.e.rw_salvar(), n = this.e.rw_salvo_tamanho();
      const texto = new TextDecoder().decode(new Uint8Array(this.mem.buffer, p, n).slice());
      this.port.postMessage({ tipo: 'salvo', destino: m.destino, texto, seed: this.seedAtual() });
    } else if (m.tipo === 'abrir' && this.e) {
      const b = new TextEncoder().encode(m.texto);
      new Uint8Array(this.mem.buffer, this.e.rw_entrada(b.length), b.length).set(b);
      if (this.e.rw_abrir(b.length)) { this.tocando = true; this.mandarRack('patch'); }
      else this.port.postMessage({ tipo: 'nada', acao: 'abrir' });
    } else if (m.tipo === 'visiveis') {
      this.visiveis = m.ids;
    } else if (m.tipo === 'ganho') {
      this.ganho = m.valor;
    }
  }

  // a seed do patch que soa (0 = feito à mão), como texto
  seedAtual() {
    return ((BigInt(this.e.rw_seed_alto() >>> 0) << 32n) | BigInt(this.e.rw_seed_baixo() >>> 0)).toString();
  }

  // o rack para a página desenhar: a cada seed e a cada mudança de cabos
  mandarRack(tipo) {
    const seed = this.seedAtual();
    const dec = new TextDecoder();
    const texto = dec.decode(new Uint8Array(this.mem.buffer, this.e.rw_descrever(), this.e.rw_texto_tamanho()).slice());
    const wp = this.e.rw_wordmark(), b = new Uint8Array(this.mem.buffer);
    let fim = wp; while (b[fim]) fim++;
    const wordmark = dec.decode(b.slice(wp, fim));
    this.nc = this.e.rw_num_controles();
    this.tipos = {};
    for (const md of JSON.parse(texto).modulos) this.tipos[md.id] = md.tipo;
    this.port.postMessage({ tipo, seed, cabos: this.e.rw_cabos(), texto, wordmark });
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
      const c = new Float32Array(this.mem.buffer, this.e.rw_cabos_estado(), nCab * 14).slice();
      // os visores na tela (748 números cada, `rw_visor`) e, no QUANTIZER,
      // o nome da escala do banco
      const ids = this.visiveis, VISOR = 748;
      const vis = new Float32Array(ids.length * VISOR), escalas = {};
      ids.forEach((id, k) => {
        vis.set(new Float32Array(this.mem.buffer, this.e.rw_visor(id), VISOR), k * VISOR);
        if (vis[k * VISOR + 732] > 12.5 && this.tipos?.[id] === 'QUANTIZER') {
          const p = this.e.rw_nome_escala(id), b = new Uint8Array(this.mem.buffer);
          let f = p; while (b[f]) f++;
          escalas[id] = new TextDecoder().decode(b.slice(p, f));
        }
      });
      this.port.postMessage({ tipo: 'quadro', valores: v, cabos: c, ids, visores: vis, escalas, vu: this.e.rw_vu() },
                            [v.buffer, c.buffer, vis.buffer]);
    }
    return true;
  }
}

registerProcessor('rasgo', RasgoProcessor);
