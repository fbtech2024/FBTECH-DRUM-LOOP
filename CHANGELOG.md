# CHANGELOG — FBTECH DRUM LOOP PRO

## Sviluppo MIDI e feedback visivo — 2026-10-23
- Gestione note MIDI in ingresso con mapping GM per quattro strumenti.
- Riproduzione live MIDI anche con trasporto sequencer fermo.
- Pubblicazione del passo corrente e indicatore visivo animato nella griglia.
- Da verificare CI finale, test hardware ATOM, velocity WAV e sincronizzazione host.


## Sviluppo MASTER — 2026-10-23 (non ancora release pubblica)
- Sequencer 16-step con 4 strumenti e suoni sintetici/WAV.
- Kit WAV da cartella, volumi per voce, pattern A/B/C/D, FILL NEXT BAR.
- SONG MODE con 8 slot, pattern e durata per slot, AUTO FILL, STOP AT END.
- Salvataggio/caricamento arrangiamento locale in formato .fbdl.
- MOVE LEFT / MOVE RIGHT, ADD SLOT / REMOVE SLOT.
- Build Windows e Mac Intel con GitHub Actions.
- Aggiornato STATO_PROGETTO.md con piano unico di completamento e rischi aperti.

## Prima della release
- Confermare CI sul commit finale, persistenza dei kit, compatibilità DAW e Mac Intel reale.
- Verificare licenze JUCE e campioni prima della distribuzione.
- Non dichiarare completata la release finché questi punti non sono chiusi.

## Pad virtuali — 2026-10-23
- Aggiunti quattro pad suonabili in italiano e trasferimento sicuro delle richieste dalla GUI al thread audio.
- Interfaccia ampliata preservando l'arrangiatore; build e collaudo funzionale ancora da confermare.

## Griglia 4x4 — 2026-10-23
- Espansa l'interfaccia a 16 pad cliccabili, con quattro strumenti condivisi e numerazione italiana.
- Nessuna aggiunta di voci audio indipendenti; verifica CI e usabilità ancora aperta.

## Dinamica WAV e code audio — 2026-10-23
- Aggiunto controllo velocity sui campioni WAV, mantenendo il trattamento già presente per i sintetizzatori.
- Rimossa uscita anticipata alla fine del brano che poteva tagliare code audio.
- CI e test audio reali ancora da confermare.

## Registrazione live MIDI — 2026-10-23
- Aggiunto REGISTRA PAD: cattura note MIDI e pad GUI nella griglia del pattern selezionato, quantizzate ai sedicesimi.
- Registrazione attiva solo a sequencer avviato e fuori Song Mode.
- Funzione non ancora collaudata su hardware/DAW; build CI in corso.

## MIDI Learn — 2026-10-23
- Aggiunta assegnazione MIDI Learn per ciascuna delle quattro voci e persistenza nel preset v8.
- Pulsanti GUI mostrano nota appresa; da verificare compilazione, conflitti di mapping e funzionamento ATOM.

## MIDI Learn: gestione assegnazioni — 2026-10-23
- Impedite note MIDI Learn duplicate tra le quattro voci.
- Aggiunto reset per singola voce con SHIFT+clic e istruzione nell'interfaccia.
- Build nuove da verificare; controller fisico non ancora collaudato.

## Fallback MIDI e preset — 2026-10-23
- Soppressi alias General MIDI per le voci rimappate; risolto il conflitto con note apprese.
- Validazione delle assegnazioni duplicate quando si ripristina un preset.
- Build e test reali ancora da confermare.

## Correzioni registrazione live — 2026-10-23
- Aggiornamento visivo dei passi registrati nella griglia 16-step.
- La registrazione usa il banco attualmente in riproduzione, non quello accodato.
- Verificata architettura quattro voci prima di avviare la migrazione a sedici.

## Dinamica dei 16 pad GUI — 2026-10-23
- Quattro intensità di colpo per ognuna delle quattro voci, mantenendo compatibilità con il motore attuale.
- Espansione a 16 voci indipendenti ancora non implementata.

## Rifinitura interfaccia — 2026-10-23
- Etichette pad aggiornate con percentuali di velocity.
- Secondo clic sul MIDI Learn armato annulla l'apprendimento senza cancellare la nota salvata.

## Fine Song Mode — 2026-10-23
- Corretto il taglio delle code audio alla fine della song; niente nuovi step dopo STOP AT END, rendering WAV/synth continua.
- Nuova build da verificare.

## Inizio migrazione eventi 16 pad — 2026-10-23
- `pendingPadHits` espanso da quattro a sedici slot atomici, uno per pad.
- I pad grafici passano il proprio indice al motore; il rendering conserva temporaneamente quattro voci per compatibilità.
- Nessuna migrazione del formato preset in questa fase.

## 16 slot WAV — 2026-10-23
- Nuovi 16 slot indipendenti di campioni WAV, code e rendering; SHIFT+clic sul pad per caricare il WAV.
- Preset formato 9 con campioni incorporati per tutti i pad, compatibilità lettura preset precedenti.
- Nomi dei campioni mostrati alla riapertura editor.
- Sequencer/MIDI Learn restano a quattro voci; build e test da verificare.

## Sequencer a 16 tracce — 2026-10-23
- Espansi pattern e playback a 16 tracce, editor con quattro pagine di quattro tracce.
- Preset versione 10 con migrazione dei vecchi pattern da quattro tracce.
- Restano da verificare build e test host; MIDI Learn a 16 pad non ancora implementato.

## MIDI Learn per tutti i pad — 2026-10-23
- Sedici note MIDI assegnabili, selezionando la pagina del sequencer e usando MIDI Learn sulla relativa riga.
- Mappature incluse nei preset v11, mantenendo lettura delle precedenti versioni.
- Compilazioni e test con hardware reale da verificare.

## MIDI Learn: protezioni UI — 2026-10-23
- Disattiva il MIDI Learn armato quando si cambia pagina sequencer.
- Scarta eventi MIDI a velocità zero nell'apprendimento.
- Build e test hardware ancora da verificare.

## MIDI CC Learn quattro volumi — 2026-10-23
- ALT+clic sul MIDI LEARN delle quattro voci principali (pagina 1) per apprendere il CC di una manopola; il CC controlla il volume mixer 0–1,5.
- Slider sincronizzati, mappature CC univoche e salvate nei preset v12.
- Non verificato su hardware reale; nessun feedback LED o controllo trasporto ATOM.

## Riduzione code CI — 2026-10-23
- Workflow Windows e macOS Intel aggiornati con concorrenza per workflow/branch e annullamento delle compilazioni superate dai commit successivi.
- Restano da verificare i run del MASTER finale e il comportamento in DAW.

## Feedback import WAV — 2026-10-23
- Messaggi di successo/errore visibili e persistenti dopo il caricamento WAV dei singoli pad; annullamento dialogo ignorato.
- Test CI e DAW da verificare.

### 2026-10-23 — Separazione interfaccia
- Introdotte tre viste SUONA / EDITOR RITMI / ARRANGIAMENTO e ridotta altezza plugin; motore audio invariato.
- Mancano libreria ritmi e allineamento grafico al sito; CI e collaudo host in attesa.

- UI skin JUCE personalizzata (LookAndFeel_V4): pulsanti arrotondati, stati hover/pressed, palette blu scuro/nero/bianco, menu e slider coerenti. Commit 017c8a8e; build e test DAW non ancora verificati.
- Ricerca UX: Addictive Drums 2 usa navigazione per pagine e browser groove con anteprima; la libreria groove FBTECH e' ancora da implementare, senza inventare contenuti.

## UI FBTECH DRUMS LOOP e prima libreria reale (2026-10-23)
- Navigazione implementata nel codice: RITMI, BATTERIA, EDITOR, ARRANGIAMENTO, IMPOSTAZIONI. Colori blu scuro, nero, bianco e ciano ispirati allo screenshot fornito dall'utente; interfaccia JUCE disegnata e non screenshot incorporato.
- Browser con filtri per 9 generi e 13 groove effettivi offline (sequenze di 16 step per le 4 voci synth). ASCOLTA carica il groove nel pattern A e avvia il motore; CARICA RITMO sostituisce il pattern A. Per sicurezza occorre fermare il trasporto prima di cambiare groove.
- I 13 groove sono pattern ritmici programmati, NON campioni acustici professionali, non sono i 520 ritmi illustrativi dell'anteprima. Kit WAV caricabili separatamente. Il tempo segue la DAW.
- Limitazioni: grafica semplificata rispetto al mockup fotografico, nessuna fotografia incorporata; le variazioni A-D non sono generate automaticamente dal browser; ricerca testuale, anteprime audio pre-renderizzate, e libreria estesa non ancora implementate.
- Commit UI 2156d127; compilazione Windows/Mac Intel e prova in FBTECH Music ancora da verificare. Motore DSP non modificato.

- Introdotto marchio FBTECH disegnato direttamente in JUCE (FB ciano, TECH bianco, sottotitolo), come nel riferimento visivo dell'utente. Corretta la preview: ASCOLTA non avvia il motore se il groove non e' stato caricato (ad esempio trasporto in corso). Build e test DAW in attesa.

## 2026-10-23 — variazioni ritmiche A-D
- Ogni groove incorporato ora carica quattro pattern distinti: A base, B hi-hat aggiuntivi, C colpi di cassa sincopati, D fill di tom e rullante. Tutte le variazioni sono modificabili nell'editor e funzionano offline. Il caricamento sostituisce i quattro pattern correnti (solo a trasporto fermo): salvare prima il progetto se si vogliono conservare le modifiche. I pad 5-16 non vengono mantenuti nel nuovo preset. Questa e' una prima logica deterministica di variazione, non un generatore musicale intelligente. Compilazione e test host della modifica ancora da verificare.

## 2026-10-23 — guida variazioni e sicurezza anteprima
- La schermata RITMI ora mostra la legenda A base, B hi-hat, C cassa, D fill. Il pulsante ASCOLTA ferma l'anteprima precedente prima di sostituire i pattern. Se il callback audio non ha ancora recepito lo stop, chiede di premere ASCOLTA nuovamente; non sovrascrive pattern durante playback. Il trasporto DAW attivo blocca il caricamento. Build e test host della revisione in attesa.

## 2026-10-23 — BPM controllabile
- Aggiunto grande controllo BPM 40-240 nella schermata RITMI e opzione SYNC DAW. Quando il trasporto host non suona, il loop manuale usa il BPM impostato dall'utente; quando il trasporto host suona e SYNC DAW e' attivo, segue il BPM host. BPM e sync sono persistiti nei preset (versione stato 13). Il motore WAV esistente resta invariato: nessuna libreria acustica reale e' stata ancora aggiunta. Touch pad, equalizzazione e profondita' restano da sviluppare e verificare. Build e prova DAW di questa revisione non ancora confermate.

## 2026-10-23 — console: testi e BPM
- Ridotta la guida tecnica visibile nella console a istruzioni musicali essenziali (RITMI, BATTERIA, EDITOR). Sincronizzato il controllo BPM con lo stato salvato senza interferire durante il trascinamento. Le due build precedenti del MASTER 8e6c36c8 sono risultate riuscite su GitHub Actions; questa revisione richiede una nuova compilazione. Nessun kit acustico incluso ancora.

## 2026-10-23 — pad touchscreen
- Nella vista BATTERIA ingranditi i 16 pad da 220x73 a 220x87 pixel, con maggiore spaziatura verticale; riallineati caricamento kit, fill e registrazione. La risposta al tocco e il comportamento musicale non sono stati ancora provati su hardware touch. I pad restano suoni sintetizzati se non sono caricati WAV reali. Build della revisione in attesa.

## 2026-10-23 — superficie console pad
- Vista BATTERIA: aggiunta una superficie grafica a gradiente tipo metallo satinato, cornice e separatori per la griglia dei 16 pad touchscreen; componenti e funzioni restano quelli del MASTER. Modifica grafica effettiva JUCE, non un mockup. La grafica complessiva non e' ancora una console definitiva; kit WAV acustici non inclusi. Compilazione di questo commit da verificare.

## 2026-10-23 — controllo tono audio
- Implementato un filtro passa-basso master sul segnale drum (250-20000 Hz circa), regolato dalla nuova slider TONO SCURO/BRILLANTE in RITMI. Valore 1 bypass, 0 taglia gli acuti; impostazione salvata nei preset (stato versione 14). Non e' un equalizzatore multibanda o un controllo di riverbero/profondita'. Le build Windows e Mac Intel del precedente MASTER 47a5291a sono riuscite; la nuova revisione richiede compilazione e test di ascolto. Ancora nessuna libreria di campioni acustici reali integrata.
