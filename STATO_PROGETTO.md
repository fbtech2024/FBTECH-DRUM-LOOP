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
- Mac Intel: compilazione VST3 riuscita al commit 81ec3f7c; test reale in DAW macOS non confermato.
- I WAV sono ora incorporati nei preset XML v7 come PCM mono Base64; non ancora collaudata la riapertura in DAW. I preset possono risultare grandi.
- Validazione preset importati e gestione errori richiedono hardening.
- I pattern sono ora atomici; rimangono da verificare le prestazioni realtime di caricamento campioni e serializzazione.
- Comportamento del trasporto/STOP AT END in host diversi da verificare.
- Licenza JUCE e diritti dei campioni da definire prima di distribuzione pubblica.

## CI e correzioni recenti
- Windows e macOS Intel: build riuscite al commit 81ec3f7c.
- Modifiche successive: 173297bb (validazione rate), 6479eecf (overflow step), 48529467 (BPM), c6d3ec3c (downmix stereo); verificare esito CI.
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
