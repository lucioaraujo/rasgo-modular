## O que é
LISTEN recebe um som e acompanha sua energia em três regiões: grave, médio e agudo. Cada região oferece um nível contínuo e um pulso quando detecta um ataque. Você pode usar esses sinais para fazer outra voz responder ao que está tocando.

## Como pensar nele
Uma batida grave pode chamar uma nota; os pratos podem mudar o brilho de uma voz. As regiões se sobrepõem: um mesmo instrumento pode aparecer em mais de uma saída. LISTEN mede frequências e contrastes de energia, sem separar instrumentos ou descobrir sozinho o andamento da música. THRU conserva o som recebido para você também ouvi-lo. Os medidores mostram os níveis e destacam ataques recentes.

## Controles
- LOW: separação entre grave e médio, de 60 a 800 Hz. Após mover um corte, os ataques esperam 50 ms para evitar disparos causados pelo ajuste. Uma modulação contínua pode mantê-los suspensos.
- HIGH: separação entre médio e agudo, de 1 a 8 kHz, limitada pela taxa de amostragem.
- RESP: suaviza o movimento dos níveis. Aumente para que a energia permaneça depois de cada golpe. Não muda a detecção de ataques.
- SENS: sensibilidade aos ataques. Aumente para perceber contrastes menores; diminua se um golpe provocar eventos demais.
- FLOOR: piso de amplitude para disparar, de −72 a −18 dBFS. Subir esse piso ajuda a rejeitar ruído fraco.
- REARM: espera mínima entre ataques da mesma região, de 20 a 250 ms. Um valor alto pode deixar golpes rápidos de fora.
Entrada: IN, o áudio. Saídas: THRU, o áudio preservado por canal; LOW, MID e HIGH, níveis de 0 a 1; L-HIT, M-HIT e H-HIT, pulsos de 5 ms. A calibração fica fora do movimento automático do VARIA.

## Experimente
1. Desligue VARIA. Ligue CLOCK.EUC a DRUM.GATE, DRUM.OUT a LISTEN.IN e LISTEN.THRU a uma entrada do MIXER. Toque em volume baixo e observe os medidores.
2. Ligue L-HIT a PHRASE.CLK. Para ouvir a resposta, ligue PHRASE.PITCH a QUANTIZER.CV, com RANGE=1; QUANTIZER.PITCH a OSC.PITCH; OSC.SAW a ENVELOPE.IN; PHRASE.GATE a ENVELOPE.GATE; ENVELOPE.OUT ao MIXER.
3. Mude TONE e SNAP no DRUM. Ajuste FLOOR e REARM até a voz responder aos golpes desejados. Troque L-HIT por H-HIT e ouça como muda o diálogo.
4. Para uma resposta com intensidade variável, ponha um VCA entre ENVELOPE e MIXER, com LVL1=0 e CV1=1; ligue PHRASE.ACC a VCA.CV1 e LISTEN.LOW a PHRASE.STIM. Se usar áudio externo, envie SIGNAL-IN.OUT a LISTEN.IN e ative ON no SIGNAL-IN por um gesto seu.
