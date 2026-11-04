# Orage v5 — Foudre Audio

Deux plugins, même design (métal peint noir usé, lampes qui chauffent) :

- **Orage** (instrument VST3) : synthé 6 voix façon Korg MS20-M + tout le rack d'effets d'Orage FX. Deux onglets : SYNTH / EFFECTS. Réponds au MIDI. Clavier à l'écran en bas.
- **Orage FX** (effet VST3) : le rack d'effets seul, pour les pistes audio.

## Synthé
- 2 VCO (saw / pulse / triangle), octaves, tune/fine, PWM, **hard sync**, **FM** (VCO2 → VCO1), **ring mod**, bruit, glide.
- Filtres MS20 en série HPF → LPF avec résonance (auto-oscillation), drive, key tracking, enveloppe de filtre.
- Enveloppes ADSR amp et filtre.
- **4 LFO** façon Serum : formes SIN / TRI / SAW / SQR / **S&H** (avec SLEW pour lisser les marches), aperçu de la forme, FREQUENCY, FADE-IN, RETRIG, synchro tempo Ableton.
- **4 macros** et **matrice de modulation à 8 slots** (sources : LFO 1-4, macros 1-4, env filtre, vélocité, molette ; destinations : pitch, pitch VCO2, PWM 1/2, FM, ring, mix VCO2, HPF, LPF, reso, level, pan).
- Molette de pitch (±2 demi-tons) et mod wheel (CC1).

## Effets (onglet EFFECTS / Orage FX)
Rack réordonnable, **prises incrustées sur la face de chaque module** (câbles fins, bouton CABLES pour les masquer, survol d'une prise = câble en surbrillance), Gate, Octaver, Distorsion, Color, Filtre, Destroy, Tube Comp (ATTACK/THRESHOLD/VU), Spring, LFO/S&H/Envelope, presets, trois remises à zéro.

## Compilation (GitHub Actions)
1. Remplace dans ton dépôt `CMakeLists.txt` et le dossier `Source/` par ceux de ce zip.
2. **Remplace aussi le contenu de `.github/workflows/build.yml`** (il récupère maintenant les deux plugins).
3. Lance le workflow, télécharge l'artefact `Orage-VST3-Windows` : dossiers `Orage.vst3` et `Orage FX.vst3` à copier dans `C:\Program Files\Common Files\VST3`.
4. Dans Ableton : Orage se glisse sur une piste MIDI, Orage FX sur une piste audio.

## Design
Façade façon Korg MS20 : zones délimitées par des traits blancs sérigraphiés, potards numérotés 0-10, symboles de formes d'onde. Les fenêtres de confirmation (vider le rack, patchbay, reset, sauvegarde) sont intégrées au plugin. La fenêtre se redimensionne.
