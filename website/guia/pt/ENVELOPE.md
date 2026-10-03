## O que é
O ENVELOPE dá forma a uma nota no tempo: como ela começa, como cai, quanto se sustenta e como some. Ele recebe um pulso na entrada GATE e desenha uma curva a cada pulso. Já traz um controle de volume embutido, então basta passar um som por ele para ouvir notas no lugar de um som contínuo.

## Como pensar nele
As fontes soam o tempo todo; o ENVELOPE transforma esse fluxo em frases. Ele tem dois usos que podem acontecer ao mesmo tempo. Pela entrada IN e saída OUT, ele articula o som que passa. Pela saída ENV, ele entrega a curva em si, que você pode mandar para abrir um filtro, mudar um timbre ou dosar um efeito. As quatro etapas da curva têm nomes em inglês que aparecem em todo sintetizador: attack, decay, sustain e release, por isso ADSR.

## Controles
- ATK: o ataque, o tempo que a nota leva para chegar ao máximo depois do pulso. Curto soa percussivo; longo, como um arco que entra devagar.
- DEC: o decaimento, o tempo da queda depois do máximo até o nível de SUS.
- SUS: o nível em que a nota se sustenta enquanto o pulso continua alto. Em zero, não há sustentação e a nota soa como algo dedilhado.
- REL: o tempo que a nota leva para sumir depois que o pulso termina.
- CURVE: o formato das curvas, de mais suave a mais incisivo, com o ataque percebido mais cedo.
- TRIG: em uma posição, a nota se sustenta enquanto o pulso durar; na outra, cada pulso dispara a curva inteira, sem esperar.
- VCA: quanto o volume embutido age sobre o som que passa. Em zero, o som atravessa sem mudar e só a saída ENV importa.
- LVL: o volume de saída.
Entradas: IN, o som; GATE, o pulso que dispara a nota; TIME, para acelerar ou retardar todas as etapas. Saídas: OUT, o som articulado; ENV, a curva.

## Experimente
1. Ligue a saída SAW de um OSC na entrada IN, OUT numa entrada do MIXER, e a saída EUC do CLOCK em GATE. Você ouve notas.
2. Suba ATK e ouça cada nota entrar devagar. Volte e mexa em DEC para deixá-las curtas ou longas.
3. Ponha um FILTER entre o OSC e o ENVELOPE, com CUT baixo, e ligue ENV na entrada FC do filtro. Agora cada nota também abre o timbre.
