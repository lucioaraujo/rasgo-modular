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

// TextDecoder/TextEncoder não existem dentro de um AudioWorklet (nem no
// Firefox nem no Chrome): o UTF-8 é feito aqui. Sem isto, a descrição do
// rack não saía do worklet — o som tocava e a página ficava sem módulos.
function deUtf8(b) {
  let s = '', parte = [];
  for (let i = 0; i < b.length;) {
    const c = b[i++];
    let u;
    if (c < 0x80) u = c;
    else if (c < 0xe0) u = ((c & 0x1f) << 6) | (b[i++] & 0x3f);
    else if (c < 0xf0) u = ((c & 0x0f) << 12) | ((b[i++] & 0x3f) << 6) | (b[i++] & 0x3f);
    else u = ((c & 0x07) << 18) | ((b[i++] & 0x3f) << 12) | ((b[i++] & 0x3f) << 6) | (b[i++] & 0x3f);
    if (u > 0xffff) { u -= 0x10000; parte.push(0xd800 + (u >> 10), 0xdc00 + (u & 0x3ff)); }
    else parte.push(u);
    if (parte.length > 8000) { s += String.fromCharCode.apply(null, parte); parte = []; }
  }
  return s + String.fromCharCode.apply(null, parte);
}
function paraUtf8(t) {
  const out = [];
  for (let i = 0; i < t.length; i++) {
    let u = t.charCodeAt(i);
    if (u >= 0xd800 && u < 0xdc00 && i + 1 < t.length) u = 0x10000 + ((u - 0xd800) << 10) + (t.charCodeAt(++i) - 0xdc00);
    if (u < 0x80) out.push(u);
    else if (u < 0x800) out.push(0xc0 | (u >> 6), 0x80 | (u & 63));
    else if (u < 0x10000) out.push(0xe0 | (u >> 12), 0x80 | ((u >> 6) & 63), 0x80 | (u & 63));
    else out.push(0xf0 | (u >> 18), 0x80 | ((u >> 12) & 63), 0x80 | ((u >> 6) & 63), 0x80 | (u & 63));
  }
  return new Uint8Array(out);
}
const decoder = { decode: deUtf8 };

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
      // o gesto inteiro vira um evento na partitura do REC (`onParamGesture`)
      if (this.arrasto) this.e.rw_rec_param(this.arrasto.i, this.arrasto.v0, this.e.rw_param_usuario(this.arrasto.i));
      this.arrasto = null;
      this.e.rw_mao(0);
    } else if (m.tipo === 'mao' && this.e) {
      this.e.rw_mao(m.ativa ? 1 : 0);
    } else if (m.tipo === 'alterna' && this.e) {
      const de = this.e.rw_param_usuario(m.i), para = de >= 0.5 ? 0 : 1;
      this.e.rw_param(m.i, para);
      this.e.rw_rec_param(m.i, de, para);
    } else if (m.tipo === 'roda' && this.e) {
      const de = this.e.rw_param_usuario(m.i), para = valorRoda(de, m.dir, m.fino, m.lo, m.hi);
      this.e.rw_param(m.i, para);
      this.e.rw_rec_param(m.i, de, para);
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
      const texto = deUtf8(new Uint8Array(this.mem.buffer, p, n).slice());
      this.port.postMessage({ tipo: 'salvo', destino: m.destino, texto, seed: this.seedAtual() });
    } else if (m.tipo === 'abrir' && this.e) {
      const b = paraUtf8(m.texto);
      new Uint8Array(this.mem.buffer, this.e.rw_entrada(b.length), b.length).set(b);
      if (this.e.rw_abrir(b.length)) { this.tocando = true; this.mandarRack('patch'); }
      else this.port.postMessage({ tipo: 'nada', acao: 'abrir' });
    } else if (m.tipo === 'adicionar' && this.e) {
      const id = this.e.rw_adicionar(this.texto(m.modulo));
      if (id >= 0) this.mandarRack('rack', id);
      else this.port.postMessage({ tipo: 'recusa', modulo: m.modulo, motivo: id });
    } else if (m.tipo === 'remover' && this.e) {
      if (this.e.rw_remover(m.id, m.foto === false ? 0 : 1)) this.mandarRack('rack');
    } else if (m.tipo === 'ordem' && this.e) {
      if (this.e.rw_ordem(this.texto(m.ids.join(' ')))) this.mandarRack('rack');
    } else if (m.tipo === 'escala' && this.e) {
      this.e.rw_escolher_escala(m.id, m.banco);
    } else if (m.tipo === 'lerBanco' && this.e) {
      this.port.postMessage({ tipo: 'bancoAtual', id: m.id, banco: this.e.rw_parametro(m.id, this.texto('bank')) });
    } else if (m.tipo === 'midi' && this.e) {
      this.e.rw_midi(m.d[0], m.d[1], m.d[2] ?? 0);
    } else if (m.tipo === 'rec' && this.e) {
      if (m.ligar && !this.gravando) this.iniciarRec();
      else if (!m.ligar && this.gravando) this.terminarRec();
    } else if (m.tipo === 'compacto' && this.e) {
      this.e.rw_compacto(m.ligado ? 1 : 0);
      this.mandarRack('redesenho');
    } else if (m.tipo === 'lingua' && this.e) {
      this.e.rw_lingua(m.l);
    } else if (m.tipo === 'learnAlvo') {
      this.learnAlvo = m.id >= 0 ? m : null;
    } else if (m.tipo === 'visiveis') {
      this.visiveis = m.ids;
    } else if (m.tipo === 'ganho') {
      this.ganho = m.valor;
    }
  }

  // ---- REC: o som da saída do motor (antes do volume da página), em blocos
  // de 1 s mandados à página; a partitura fica no motor (rw_rec_*)
  iniciarRec() {
    this.e.rw_rec_iniciar();
    this.gravando = true;
    this.recQuadros = 0;
    this.novoBlocoRec();
  }
  // ~1 s, múltiplo exato do quantum (256 números = 128 quadros L R)
  novoBlocoRec() { this.recBloco = new Float32Array(256 * Math.round(sampleRate / 128)); this.recPos = 0; }
  mandarBlocoRec() {
    if (!this.recPos) return;
    const b = this.recBloco.slice(0, this.recPos);
    this.port.postMessage({ tipo: 'recAudio', bloco: b }, [b.buffer]);
    this.novoBlocoRec();
  }
  terminarRec() {
    this.gravando = false;
    this.mandarBlocoRec();
    const seed = (BigInt(this.e.rw_rec_seed_alto() >>> 0) << 32n) | BigInt(this.e.rw_rec_seed_baixo() >>> 0);
    // o nome como no app: rec-data-hora-seedN (ou -manual), -montage se a
    // seed mudou durante a tomada
    const d = new Date(), z = (n, k = 2) => String(n).padStart(k, '0');
    const nome = `rec-${d.getFullYear()}${z(d.getMonth() + 1)}${z(d.getDate())}-${z(d.getHours())}${z(d.getMinutes())}${z(d.getSeconds())}`
      + (seed ? `-seed${seed}` : '-manual') + (this.e.rw_rec_montagem() ? '-montage' : '');
    const tz = -d.getTimezoneOffset();
    const data = `${d.getFullYear()}-${z(d.getMonth() + 1)}-${z(d.getDate())} ${z(d.getHours())}:${z(d.getMinutes())}:${z(d.getSeconds())} `
      + `${tz < 0 ? '-' : '+'}${z(Math.floor(Math.abs(tz) / 60))}${z(Math.abs(tz) % 60)}`;
    const seg = (this.recQuadros / sampleRate).toFixed(1);
    const vp = this.e.rw_versao(), u8 = new Uint8Array(this.mem.buffer); let f = vp; while (u8[f]) f++;
    const versao = deUtf8(u8.slice(vp, f));
    const cab = `${data}\t${nome}.wav (24 bits, ${sampleRate} Hz, ${seg} s)\tRasgo Modular ${versao} (navegador)`;
    const sp = this.e.rw_rec_parar(this.texto(cab)), u = new Uint8Array(this.mem.buffer);
    let g = sp; while (u[g]) g++;
    const partitura = deUtf8(u.slice(sp, g));
    const midi = new Uint8Array(this.mem.buffer, this.e.rw_rec_midi(), this.e.rw_rec_midi_tamanho()).slice();
    this.port.postMessage({ tipo: 'recFim', nome, partitura, midi, taxa: sampleRate, quadros: this.recQuadros }, [midi.buffer]);
  }

  // um texto na entrada do motor (`rw_entrada`); devolve o tamanho
  texto(t) {
    const b = paraUtf8(t);
    new Uint8Array(this.mem.buffer, this.e.rw_entrada(b.length), b.length).set(b);
    return b.length;
  }

  // a seed do patch que soa (0 = feito à mão), como texto
  seedAtual() {
    return ((BigInt(this.e.rw_seed_alto() >>> 0) << 32n) | BigInt(this.e.rw_seed_baixo() >>> 0)).toString();
  }

  // o rack para a página desenhar: a cada seed e a cada mudança de cabos
  mandarRack(tipo, novo = -1) {
    const seed = this.seedAtual();
    const dec = decoder;
    const texto = dec.decode(new Uint8Array(this.mem.buffer, this.e.rw_descrever(), this.e.rw_texto_tamanho()).slice());
    const wp = this.e.rw_wordmark(), b = new Uint8Array(this.mem.buffer);
    let fim = wp; while (b[fim]) fim++;
    const wordmark = dec.decode(b.slice(wp, fim));
    this.nc = this.e.rw_num_controles();
    this.tipos = {};
    for (const md of JSON.parse(texto).modulos) this.tipos[md.id] = md.tipo;
    this.port.postMessage({ tipo, novo, seed, cabos: this.e.rw_cabos(), texto, wordmark });
  }

  process(inputs, outputs) {
    const out = outputs[0];
    if (!this.e || !this.tocando || out.length < 2) return true;
    // o microfone (a página só o liga quando um SIGNAL-IN pede): mono vira
    // L = R; entregue antes do bloco, como a placa de som no app
    const ent = inputs[0];
    if (ent && ent.length) {
      const a = ent[0], b = ent[1] || ent[0], buf = new Float32Array(this.mem.buffer, this.e.rw_audio_buffer(), 256);
      for (let i = 0; i < a.length; i++) { buf[2 * i] = a[i]; buf[2 * i + 1] = b[i]; }
      this.e.rw_audio_in(a.length);
    }
    const p = this.e.rw_processar();
    const f = new Float32Array(this.mem.buffer, p, 256);
    const L = out[0], R = out[1], g = this.ganho;
    for (let i = 0; i < L.length; i++) { L[i] = f[2 * i] * g; R[i] = f[2 * i + 1] * g; }
    if (this.gravando) {
      this.recBloco.set(f, this.recPos);
      this.recPos += 256; this.recQuadros += 128;
      if (this.recPos >= this.recBloco.length) this.mandarBlocoRec();
      if (!this.e.rw_rec_ativo()) this.terminarRec();   // os 4 minutos
    }
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
          escalas[id] = deUtf8(b.slice(p, f));
        }
      });
      // LEARN: o valor ao vivo do controle sob o mouse
      let learnValor = '';
      if (this.learnAlvo) {
        const p = this.e.rw_learn_valor(this.learnAlvo.id, this.texto(this.learnAlvo.bind)), u = new Uint8Array(this.mem.buffer);
        let f = p; while (u[f]) f++;
        learnValor = deUtf8(u.slice(p, f));
      }
      this.port.postMessage({ tipo: 'quadro', valores: v, cabos: c, ids, visores: vis, escalas, vu: this.e.rw_vu(), learnValor, entrada: this.e.rw_entrada_necessaria(), rec: this.gravando ? this.recQuadros / sampleRate : -1,
                              lufs: Array.from(new Float32Array(this.mem.buffer, this.e.rw_lufs(), 4)) },
                            [v.buffer, c.buffer, vis.buffer]);
    }
    return true;
  }
}

registerProcessor('rasgo', RasgoProcessor);
