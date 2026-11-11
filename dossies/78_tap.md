# 78 — TAP: o andamento batido por quem toca

Data: 11/11/2026. Estado: **em implementação** (estudo da família §4.9 e
§4.12, aprovado: "FRASE e TAP — a escuta do gesto de quem toca"; "pode
avançar"). Origem: `tap_tempo.py` do Maravi (mesmo autor; conceito, não
código).

## Portão conceitual

1. **Território.** O tap tempo: o músico bate o pulso, e a máquina mede. No
   Maravi foi ideia do autor: "liberar o áudio para detectar uma sequência
   de batidas feitas pelo músico".
2. **Tensão.** O CLOCK do Modular tem um BPM de knob. Quem toca por cima
   precisa que o patch **entre no seu andamento**, e no seu "um".
3. **Unidade.** O **pulso** e o **tempo forte**.
4. **Percebe / conserva / transforma / recusa / esquece.**
   - Percebe batidas.
   - Conserva as últimas 8 da contagem.
   - Transforma em andamento (a mediana dos intervalos) e fase (o "um").
   - Recusa contagens irregulares: "melhor falhar com mensagem clara do que
     devolver um andamento inventado".
   - Esquece: uma pausa longa fecha a contagem.
5. **Regra, recurso, risco, limite.**
   - Regra: o andamento vale a partir de 3 batidas (dois intervalos, o
     mínimo para medir a regularidade), com dispersão ≤ DISP e entre 20 e
     400 BPM.
   - Risco: um par solto travar num andamento qualquer, daí as 3 batidas.
   - Limite: o andamento é constante (sem rubato), como diz o próprio
     Maravi.
6. **Consequência.** O CLK do TAP no EXT do CLOCK: o patch inteiro passa a
   andar no pulso de quem bateu. O UM entra no RST dos sequenciadores.
7. **Diferença no Modular.** O CLOCK estima o período de um relógio
   externo, mas não reconhece uma contagem, não recusa, não acha o "um".
8. **Diferença para o Maravi.** Lá, as batidas estão no áudio gravado e o
   corte separa a contagem da melodia. Aqui, ao vivo, por gatilho (um
   SIGNAL-IN → SCOPE.ONS, uma tecla MIDI, um botão), e o relógio continua
   sozinho depois.
9. **Escuta.**
   - Êxito: bater 4 vezes e o patch entrar no andamento, com o "um" no
     lugar.
   - Fracasso: travar num andamento que ninguém bateu.
10. **Proveniência.** Maravi (mesmo autor). Mediana e dispersão: código
    próprio.

## Comportamento

- **A contagem:** batidas a menos de 2,5 × a mediana (e de 3 s) da
  anterior continuam a contagem; mais longe, começa outra.
- **O andamento:** a mediana dos intervalos (até 8 batidas). A dispersão é
  o maior desvio relativo à mediana.
- **A fase:** a 1ª batida da contagem é o "um". O relógio segue na fase da
  última batida: CLK a cada tempo, UM a cada COMP tempos (1–16).
- **Saídas:** BPM (CV, bpm/300) e TRAV (gate quando há andamento). O RST
  esquece tudo.

**Teste** (`test_tap`):
- 4 batidas a 0,5 s dão 120,00 BPM;
- o relógio segue regular;
- o UM cai aos 3,0 s (a 1ª batida em 1,0 s + 4 tempos);
- batidas irregulares (0,5 / 0,3 / 0,4 s com DISP 0,2) não travam;
- uma contagem nova depois de uma pausa troca para 90,00 BPM.
