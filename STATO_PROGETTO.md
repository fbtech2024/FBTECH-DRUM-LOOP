# FBTECH DRUM LOOP PRO — STATO MASTER
Aggiornamento: 2026-10-23
Repository unico: https://github.com/fbtech2024/FBTECH-DRUM-LOOP
Branch unico di sviluppo: main

## Confermato dall'utente in DAW Windows
- Plugin VST3 caricato, sequencer 16 step, 4 strumenti, kit WAV, volumi.
- Pattern A/B/C/D, cambio quantizzato, fill manuale.
- SONG MODE, AUTO FILL, STOP AT END.
- Arrangiatore fino a 8 slot, pattern e battute configurabili.
- Preset .fbdl SAVE SONG / LOAD SONG.
- Spostamento sezioni: riscontro positivo utente (verificare commit esatto installato).

## Implementato ma non ancora certificato come release
- ADD SLOT / REMOVE SLOT: commit 3e97781e, compilazioni avviate, collaudo finale non confermato.
- Mac Intel: compilazione VST3 riuscita anche al commit dec51f51; test reale in DAW macOS non confermato.
- I WAV sono ora incorporati nei preset XML v7 come PCM mono Base64; non ancora collaudata la riapertura in DAW. I preset possono risultare grandi.
- Import WAV e preset rafforzato con controlli di dimensione, sample rate e valori PCM; test di regressione ancora da completare.
- I pattern sono ora atomici; rimangono da verificare le prestazioni realtime di caricamento campioni e serializzazione.
- Comportamento del trasporto/STOP AT END in host diversi da verificare.
- Licenza JUCE e diritti dei campioni da definire prima di distribuzione pubblica.

## CI e correzioni recenti
- Windows e macOS Intel: build riuscite per il medesimo commit dec51f51.
- Windows: build riuscita anche sul commit 2a7786ae; macOS Intel dello stesso commit ancora in esecuzione al momento della verifica.
- Ultime modifiche: validazione WAV, protezione dimensione preset, sincronizzazione PPQ DAW e messaggi UI persistenti.
- Nessuna release finale o collaudo completo dichiarato.

## Piano di chiusura: un solo ciclo di release
1. Stabilizzare e verificare CI Windows/macOS del commit finale.
2. Rendere persistenti i kit WAV (preferibilmente preset portabili, senza dipendere da percorsi locali).
3. Proteggere stato/pattern e import da dati corrotti, evitando blocchi del thread audio.
4. Verificare riapertura progetto DAW, tempo/trasporto, preset e arrangiamento completo.
5. Verificare il plugin in una DAW Mac Intel reale.
6. Controllare packaging, licenza JUCE e licenze dei sample; pubblicare solo dopo questi controlli.

## Regole
- Nessun progetto parallelo, nessun nuovo branch, nessuna ricostruzione da zero.
- Non confondere build riuscita con test reale.
- Non richiedere all'utente microtest a ogni commit: preparare una build candidata finale e una checklist unica.
- Aggiornare questo file e CHANGELOG.md prima di ogni rilascio.

## Sviluppo MIDI e playhead (2026-10-23)
- MIDI note-on GM gestito: cassa 35/36, rullante 38/40, charleston 42/44/46, tom 41/43/45/47/48/50.
- Percussioni MIDI ora attivabili anche con sequencer fermo; velocity applicata agli inviluppi sintetici, da estendere ai WAV.
- Passo corrente esposto all'editor e evidenziato in riproduzione.
- Build Windows commit 9c821136 riuscita; CI ultimo commit in verifica. Nessun collaudo ATOM reale ancora effettuato.
- Mancano mappatura configurabile, comandi/LED ATOM, registrazione MIDI, pad virtuali, nuova grafica FBTECH Music e localizzazione completa.

## Pad virtuali — sviluppo 2026-10-23
- Quattro pad GUI CASSA/RULLANTE/CHARLESTON/TOM collegati al motore tramite richieste atomiche.
- Attivazione senza trasporto, preservando la sezione arrangiatore precedente.
- Ultimo commit funzionale 4be80c2f: build CI da verificare; test audio GUI e hardware non ancora eseguiti.
- Nota tecnica: i pad sono al momento quattro, non ancora una matrice di sedici; la pressione è un click, non velocity sensibile.

## Griglia pad 4x4 — 2026-10-23
- 16 pad GUI cliccabili in quattro righe, collegati alle quattro voci esistenti (quattro pad per voce).
- NON sono ancora 16 strumenti indipendenti: servono nuovi slot audio, mapping MIDI, velocity WAV e test hardware.
- Layout editor esteso; verifica di usabilità e compatibilità con finestre host ancora da eseguire.
- Build dell'ultimo commit funzionale 708cc792 da verificare.

## Correzione trasporto — 2026-10-23
- Risolto errore di stato introdotto nella gestione MIDI live: `wasPlaying` veniva azzerato prima del controllo transizione e impostato true anche a trasporto fermo.
- Commit ed70a32e: ora transizione arresto/avvio viene tracciata correttamente; compilazione e test di regressione ancora da verificare.
- Priorità: non introdurre registrazione MIDI o nuove voci prima di verificare regressioni su START/STOP, WAV live e CI.

## Dinamica campioni e fine brano — 2026-10-23
- Campioni WAV ora moltiplicati per la velocity dell'ultimo note-on MIDI/pad, senza allocazioni nel callback audio.
- Eliminata uscita anticipata a SONG FINISHED per consentire il rendering delle code audio; verificare comportamento del trasporto e arresto host.
- Commit motore 4433bd22; build Windows/macOS avviate ma non ancora certificate al momento della scrittura.
- Restano aperti test reali con ATOM, 16 strumenti indipendenti, registrazione live, MIDI Learn, rifinitura UI.

## Registrazione live base — 2026-10-23
- Pulsante REGISTRA PAD collegato a flag atomico del motore audio.
- Note MIDI e pad virtuali registrati nel pattern selezionato con quantizzazione al sedicesimo più vicino durante la riproduzione.
- Registrazione disabilitata in Song Mode per evitare modifiche impreviste all'arrangiamento.
- Questa prima implementazione registra attivazioni (on/off), non velocity per step né durata; non sono disponibili overdub undo o cancellazione selettiva.
- Da testare sincronizzazione host, latenza, note ai confini della battuta, preset, GUI e controller ATOM. Build commit 97f9031d in coda al momento della verifica.

## MIDI Learn iniziale — 2026-10-23
- Quattro controlli MIDI Learn per assegnare note fisiche alle quattro voci; mapping salvato nello stato preset XML v8, con lettura retrocompatibile dei preset v7.
- Le note GM restano fallback: la stessa nota potrebbe attivare un fallback se assegnata in modo ambiguo. Gestione conflitti e reset mapping da completare.
- Non implementati 16 mapping indipendenti, controllo knob/transport/LED ATOM, né test su hardware reale.
- Commit UI 9b16b5e0: compilazione in attesa di esito.

## MIDI Learn: conflitti e ripristino — 2026-10-23
- Compilazione Windows e Mac Intel del MIDI Learn iniziale (commit 9b16b5e0) riuscita; test con hardware ancora mancanti.
- Nuovo MIDI Learn rimuove l'eventuale precedente assegnazione della stessa nota a un'altra voce.
- SHIFT+clic sul pulsante MIDI Learn elimina l'assegnazione della voce e disarma l'apprendimento.
- Nota: il fallback General MIDI rimane attivo e può produrre conflitti semantici se una nota GM viene rimappata; richiede ulteriore verifica.
- Nuove build ancora da confermare.

## Correzione fallback MIDI — 2026-10-23
- Quando una voce ha un mapping MIDI Learn, i suoi alias General MIDI standard vengono disattivati; il mapping personalizzato prevale.
- Caricamento preset normalizza eventuali note MIDI duplicate tra voci.
- Le quattro voci continuano a condividere 16 pad grafici; ATOM, 16 voci indipendenti, registrazione e precisione richiedono collaudo hardware.
- Commit motore 1b097fcd; build e test ancora da verificare.

## Revisione registrazione e UI — 2026-10-23
- CI Windows e Mac Intel riuscite sul MASTER precedente 0fb0e31a.
- Corretto il refresh dell'editor: i colpi acquisiti dal thread audio ora aggiornano i 64 step visibili senza generare click ricorsivi.
- Registrazione ora scrive nel banco realmente in riproduzione, evitando di modificare il banco in attesa di cambio alla battuta.
- Audit delle 16 voci: architettura attuale ha array e salvataggio fissi a quattro strumenti; l'espansione richiede migrazione coordinata di WAV, pattern, mapping, preset e GUI, non è ancora implementata.
- Commit 7121735a; nuove build e test reali ancora da verificare.

## Dinamica pad virtuali — 2026-10-23
- I 16 pad virtuali offrono quattro livelli di velocity per ciascuna delle quattro voci (0.4/0.6/0.8/1.0); non sono 16 strumenti indipendenti.
- L'assegnazione usa il percorso playPad esistente, quindi nessun nuovo motore audio o formato preset.
- Build del commit e02a8f9d e test audio ancora da verificare; CI precedenti per le correzioni registrazione risultavano in corso all'ultimo controllo.

## Usabilità pad e MIDI Learn — 2026-10-23
- Ogni pad virtuale ora indica l'intensità effettiva (40%, 60%, 80%, 100%) della propria riga.
- MIDI Learn si può annullare ricliccando il pulsante armato; SHIFT+clic continua a cancellare la mappatura.
- L'architettura attuale mantiene quattro strumenti indipendenti; l'estensione a sedici non è stata ancora effettuata per evitare rotture dei preset.
- Build e test reali delle ultime modifiche da verificare.

## Correzione code audio Song Mode — 2026-10-23
- Al termine della song con STOP AT END, il processBlock non interrompe più immediatamente il buffer: le code di WAV e sintetizzatori possono decadere naturalmente.
- Non vengono avviati nuovi colpi del sequencer dopo songFinished; la riproduzione manuale MIDI resta possibile.
- La migrazione a 16 voci indipendenti NON è ancora iniziata: per evitare regressioni si sta stabilizzando il motore a quattro voci prima della modifica del formato preset.
- Commit 4c7374f1: build e test audio da verificare.

## Prima fase migrazione a 16 pad — 2026-10-23
- Implementati 16 slot atomici distinti per eventi GUI (prima erano 4), con consumo realtime senza allocazioni e inoltro compatibile alle quattro voci sonore esistenti tramite `pad % 4`.
- La GUI invia ora l'indice del pad 0..15 invece dell'indice della voce 0..3. Gli eventi distinti di pad diversi non si sovrascrivono più prima del blocco audio, pur condividendo ancora il suono della stessa voce.
- NON ancora implementati 16 campioni WAV indipendenti, 16 tracce del sequencer, né MIDI Learn per 16 voci. I preset rimangono nel formato precedente.
- Nuove build e test DAW/ATOM da verificare.
