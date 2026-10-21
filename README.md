# FBTECH DRUM LOOP

Prima base sorgente di una drum machine VST3 per Windows x64, compilata tramite GitHub Actions.

## Funzioni della base iniziale
- Sequencer a 16 step con Kick, Snare, Hi-Hat e Tom.
- Suoni sintetici interni in stile batteria.
- Lettura di tempo e posizione della DAW quando disponibili.
- Salvataggio/ripristino del pattern nello stato del plugin.
- Build Windows automatica e artifact VST3 scaricabile dalla scheda **Actions**.

## Compilazione
Ogni push su `main` avvia `.github/workflows/build-windows.yml`. Al termine apri il run riuscito e scarica l'artifact `FBTECH-DRUM-LOOP-Windows-VST3`.

## Stato
Leggi [STATO_PROGETTO.md](STATO_PROGETTO.md) e [CHANGELOG.md](CHANGELOG.md). La prima compilazione e i test in DAW devono ancora essere verificati.

## Licenza
Questa base usa JUCE 8. JUCE è disponibile secondo termini GPLv3 o licenza commerciale; prima di distribuire un plugin proprietario bisogna verificare e scegliere il percorso di licenza corretto. Il progetto non dichiara una licenza di distribuzione definitiva.
