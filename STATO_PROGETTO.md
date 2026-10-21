# STATO PROGETTO — FBTECH DRUM LOOP

**Versione:** 0.1.0 — prima base sorgente  
**Repository master:** https://github.com/fbtech2024/FBTECH-DRUM-LOOP

## Stato attuale
- [x] Repository GitHub creato e raggiungibile.
- [x] Struttura progetto JUCE/CMake per target VST3 Windows x64.
- [x] Sequencer a 16 step e quattro voci sintetiche: kick, snare, hi-hat, tom.
- [x] Salvataggio/ripristino del pattern nello stato del plugin.
- [x] Workflow GitHub Actions per build Windows e caricamento artifact.
- [ ] Prima compilazione reale su GitHub Actions.
- [ ] Correzione di eventuali errori rilevati dalla build.
- [ ] Test in una DAW Windows.
- [ ] Verifica finale della licenza per distribuzione.

## Limiti noti della prima base
- I suoni sono sintetizzati internamente; caricamento di campioni WAV non è ancora implementato.
- MIDI drag-and-drop, pattern multipli, controlli di volume per voce e uscite separate non sono ancora implementati.
- Tempo e trasporto host devono essere verificati in DAW reali.
- Una build verde prova la compilazione, non la piena compatibilità audio in ogni host.

## Prossimo passo
Caricare questi file nel repository, avviare GitHub Actions, leggere i log e correggere la prima build.
