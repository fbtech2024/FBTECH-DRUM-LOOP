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
