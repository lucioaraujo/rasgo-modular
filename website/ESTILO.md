# Como o site do Rasgo Modular fala

Decidido em 2 out. 2026, a pedido do autor, que achou o texto do site
"robótico, com muitos tiques de IA, sem muita explicação didática", e pediu
uma linguagem **séria, mas simpática e encorajadora, que estimule o
interesse e informe**. Vale para todas as páginas do site e para os quatro
idiomas. Quem escrever texto público do Modular depois lê isto antes.

## A pessoa do outro lado

Alguém curioso, que talvez nunca tenha tocado um modular. Pode ser músico
experiente, pode ser estudante. Não sabe o que é "1 V/oct", "ADSR" ou
"auto-oscilação", e não precisa saber antes de chegar: o texto explica na
primeira vez em que usa o termo. Também há quem já conheça Eurorack; para
essa pessoa, uma frase de comparação basta, sem tomar o lugar da
explicação.

## A voz

- **Frases inteiras**, no tom de quem mostra o instrumento a alguém ao
  lado. Segunda pessoa (*você*), presente.
- **Explicar antes de nomear.** Primeiro o que acontece com o som; depois,
  se ajudar, o nome técnico.
- **Concreto e audível.** "Gire devagar e ouça o agudo sumir" ensina mais
  que "controla o corte".
- **Encorajar sem exagerar.** Convidar a tentar; não prometer maravilhas.
- **Honesto sobre limites.** Se algo ainda não foi testado, dizer.

## Os tiques que este texto tinha, e que não voltam

- **"Não é X, é Y"** e variantes ("não é um módulo, é um comportamento").
  Diga o que a coisa é, direto.
- **Travessão como cola de toda frase.** Use vírgula, ponto, parênteses.
  Um travessão por parágrafo, se tanto.
- **Fórmulas no lugar de frases:** "RES no talo + cutoff baixo = seno".
  Escreva a frase.
- **Rótulos telegráficos:** "Dente/quadrada/triângulo/seno com PW".
  Diga quais formas de onda saem e o que cada uma soa.
- **Clichês:** "clássico", "orgânico", "vivo", "mágico", "o coração do
  patch", "o ponto é", "a relação é o processo".
- **Negrito espalhado.** Negrito só para o nome de um controle quando ele
  abre a explicação dele.
- **Trincas automáticas** ("rápido, simples e poderoso"). Se três coisas
  não são necessárias, use duas, ou uma.
- **Aspas de ênfase** ('wah' clássico). Aspas só para citar.

## A forma de um verbete do guia de módulos

Cada módulo tem quatro partes, nesta ordem:

1. **O que é**: dois a quatro períodos. O que o módulo faz com o som, em
   palavras simples, e para que serve num patch.
2. **Como pensar nele**: o papel dele entre os outros módulos, o que o
   torna diferente do vizinho parecido, uma ideia que ajude a usar.
3. **Controles**: cada knob ou chave do painel, pelo rótulo que aparece
   no módulo, com uma ou duas frases sobre o que muda no som. Entradas e
   saídas numa frase de resumo, ou uma linha cada quando importam.
4. **Experimente**: dois a quatro passos numerados que qualquer pessoa
   consegue seguir no app, terminando no que ouvir.

Os fatos (nomes de controles, faixas, portas) vêm do código, nunca de
memória: `build/rasgo_modular_learn_coverage --referencia` imprime a ficha
de cada módulo. A explicação mais longa de cada módulo, em português, está
em `guia/NN_nome.md`.

## Arquivos

- `guia/<idioma>/<TIPO>.md` — o verbete de cada módulo em cada idioma
  (formato abaixo). `gerar_modulos.py` monta as páginas a partir deles; o
  módulo sem arquivo cai no texto curto do LEARN do app.

```
## O que é
Parágrafo.

## Como pensar nele
Parágrafo.

## Controles
- FREQ: frase.
- RESO: frase.
Entradas e saídas: frase.

## Experimente
1. Passo.
2. Passo.
```

Os títulos das seções são sempre em português no arquivo (são chaves para
o gerador); o gerador põe o título certo de cada idioma na página.
