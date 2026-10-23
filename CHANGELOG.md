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
