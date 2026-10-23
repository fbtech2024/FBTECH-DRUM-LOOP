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
- Mac Intel: workflow presente; test reale del VST3 in DAW macOS non confermato.
- Il caricamento di un WAV non salva ancora il campione audio nel preset; chiudendo/riaprendo si rischia di perdere il kit.
- Validazione preset importati e gestione errori richiedono hardening.
- Sicurezza del pattern condiviso fra thread GUI e audio da verificare (accessi concorrenti).
- Comportamento del trasporto/STOP AT END in host diversi da verificare.
- Licenza JUCE e diritti dei campioni da definire prima di distribuzione pubblica.

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
