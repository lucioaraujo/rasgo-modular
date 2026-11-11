## O que é
O QUANTIZER prende um sinal contínuo às notas de uma escala. Um valor que sobe e desce livremente, vindo de um TURING, de um SH ou de um CHAOS, sai dele como uma sequência de notas afinadas. Na maior parte dos patches, é ele que transforma acaso em melodia.

## Como pensar nele
Sinais de controle variam de forma contínua, e notas musicais não. Sem um quantizador, uma melodia gerada por acaso cai entre as notas e soa desafinada. O QUANTIZER arredonda cada valor para a nota mais próxima da escala escolhida. Com um pulso na entrada TRIG, ele só troca de nota no pulso, e a melodia ganha ritmo. A saída GATE pulsa sempre que a nota muda de fato, o que serve para disparar um envelope só em notas novas. As entradas ROOT e SCL trocam a tônica e a escala por cabo: ligadas às saídas do HARMONY, a tonalidade da melodia muda ao longo da música.

## Controles
- SCALE: a escala, entre treze: cromática, maior, menor natural, dórica, frígia dominante, lídia, menor melódica, pentatônica menor, pentatônica maior, hirajoshi, tons inteiros, só oitavas e o crivo.
- ROOT: a tônica, a nota de partida da escala, de dó a si.
- RANGE: quantas oitavas o sinal de entrada cobre, de 1 a 6. Mais oitavas, saltos maiores.
- GLIDE: faz a altura escorregar de uma nota à seguinte.
- HYST: uma margem de tolerância. Com um sinal instável, a nota não fica pulando entre duas vizinhas.
- AFIN: a afinação, aplicada depois da escolha da nota: temperada de 12 notas (a de sempre), justa, pitagórica, mesotônica, 19 ou 31 divisões da oitava. As três históricas são medidas a partir da tônica e a acompanham quando ela muda.
- M1, R1, M2, R2: o crivo de Xenakis, usado na escala número 13. Uma nota entra se o resto da divisão por M1 for R1, ou se o resto da divisão por M2 for R2. Com M que não divide 12, a escala não se repete na oitava.
- AND: troca o ou do crivo por e: a nota precisa passar nas duas peneiras. Se nenhuma nota passar, o crivo volta ao ou, para nunca ficar mudo.
Entradas: CV, o sinal a quantizar; TRSP, para transpor antes de quantizar; TRIG, o pulso que decide quando trocar de nota; ROOT e SCL, que, com cabo, mandam na tônica e na escala no lugar dos knobs. Saídas: PTCH, a altura; GATE, um pulso a cada nota nova; ST, a nota em semitons.

## Experimente
1. Ligue a saída CV de um TURING na entrada CV do QUANTIZER, e a saída CLK do CLOCK na entrada CLK do TURING.
2. Ligue PTCH em 1V/O de um OSC, e a saída SIN do OSC num ENVELOPE ligado ao MIXER. Ligue GATE do QUANTIZER em GATE do envelope.
3. Troque SCALE e ouça a mesma sequência de valores virar outra melodia. Mude ROOT para transpor.
