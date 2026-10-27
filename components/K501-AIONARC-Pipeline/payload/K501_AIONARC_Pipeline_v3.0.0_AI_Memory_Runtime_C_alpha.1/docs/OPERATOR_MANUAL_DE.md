# Bedienhandbuch — Alpha 1 Runtime

**Status:** nicht-kanonischer Kandidat. Zuerst nur mit synthetischen Testdaten arbeiten.

## 1. Sichere Vorbereitung

Das Paket immer in ein neues, isoliertes Verzeichnis entpacken. Nicht über Alpha 0, kanonische Repositories oder reale Quellen entpacken. Quellordner und Memory-Store müssen getrennt liegen. Fehlerhafte oder unvollständige Staging-Verzeichnisse erhalten und nicht manuell reparieren.

## 2. Bauen und testen

    ./scripts/build.sh
    make test

Der Build-Vertrag setzt ein generisches x86-64-Ziel (`-march=x86-64 -mtune=generic`) voraus. `pdftotext` ist optional. Die Testsuite verwendet synthetische, temporäre Daten; sie scannt keine kanonischen K501-Verzeichnisse.

## 3. Ersten synthetischen Frame anlegen

    mkdir -p "$HOME/K501_A1_TEST/source"
    printf 'proof before state\n' > "$HOME/K501_A1_TEST/source/README.md"
    build/k501-memory init "$HOME/K501_A1_TEST/store"
    build/k501-memory scan "$HOME/K501_A1_TEST/source" "$HOME/K501_A1_TEST/store" k501-a1-test frame-001 none
    build/k501-memory verify "$HOME/K501_A1_TEST/store" frame-001
    build/k501-memory query "$HOME/K501_A1_TEST/store" latest "proof before state"

Ein Frame gilt erst dann als erfolgreich geprüft, wenn `VERIFY PASS` vorliegt. `EXIT_ZERO` eines übergeordneten Prüflaufs ist allein noch kein Gate-PASS.

## 4. Textstatus und PDF

`INVALID_UTF8` bedeutet: Der UTF-8-Validator hat die Bytes abgelehnt. `EMBEDDED_NUL` bedeutet: UTF-8 war gültig, aber NUL verhindert die sichere Textaufnahme. Beide bleiben strukturell dokumentiert und haben keine Textzeile. `CAPTURED_EMPTY` ist dagegen ein erfolgreicher Null-Byte-Textdatensatz und muss mit `TEXT.tsv` roundtrip-fähig sein.

Ohne `pdftotext` muss das PDF strukturell erhalten bleiben, der Status `PDF_EXTRACTOR_UNAVAILABLE_OR_FAILED` erscheinen und die Verifikation trotzdem funktionieren. Siehe `MEMORY_CONTRACT_V1.0.md`.

## 5. Bei Fehlern

Bei `VERIFY FAIL`, `SOURCE DRIFT`, ungültigem Index oder Proof-Fehler stoppen und vollständige Ausgabe aufbewahren. Keine Hashes nachträglich korrigieren, keine Frames überschreiben und keine Staging-Verzeichnisse automatisch löschen.
