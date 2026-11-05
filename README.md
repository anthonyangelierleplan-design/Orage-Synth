# Orage v8 — Foudre Audio

## Nouveautés v8
- **Design** : trois thèmes (CHAMPAGNE par défaut, ARGENT, CUIVRE), bouton LOOK dans l'en-tête ; potards et touches métalliques en relief, plus aucune usure ; Roulette et Chaos retirés.
- **Oscilloscope** : un clic sur la lampe du haut bascule entre la lampe (chauffe) et l'oscilloscope (sortie finale).
- **Réverb plaque** (Dattorro) avec SIZE, DECAY, PRE-DELAY, DAMPING, MOTION, WIDTH, LOW CUT, **SHIMMER** (+1 octave) et **FREEZE** ; module du rack, cibles de la matrice (MIX, DECAY, DAMP).
- **Filtre passe-bas** au choix : MS-20, SEM (12 dB, morph LP/notch/HP) ou Ladder (24 dB).
- **Matrice** : 12 slots, menus rangés par familles ; nouvelles sources (ENV AMP, KEY TRACK, RANDOM, AFTERTOUCH, POLY AT) et destinations (NOISE, HPF RESO, DRIVE, SEM MORPH, FLT ENV AMT, FOLD SYM, LEVEL VCO1, réverb).
- **Presets** : ~40 presets d'usine classés par catégorie (Pad, Bass, Lead, Pluck/Keys, Arp/Seq, FX/Texture, Rack FX) ; sauvegarde utilisateur avec catégorie.
- Les modules FX démarrent à zéro (neutres) quand on les ajoute ; les presets d'usine gardent leur son.

Deux plugins, même design (métal peint noir usé, lampes qui chauffent) :

- **Orage** (instrument VST3) : synthé 6 voix façon Korg MS20-M + tout le rack d'effets d'Orage FX. Deux onglets : SYNTH / EFFECTS + volet MATRIX. Réponds au MIDI.
- **Orage FX** (effet VST3) : le rack d'effets seul, pour les pistes audio.

## Synthé
- 2 VCO (saw / pulse / triangle), octaves, tune/fine, PWM, **hard sync**, **FM** (VCO2 → VCO1), **ring mod**, bruit, glide.
- Filtres MS20 en série HPF → LPF avec résonance (auto-oscillation), drive, key tracking, enveloppe de filtre.
- Enveloppes ADSR amp et filtre.
- **4 LFO** (un seul affiché, choisi dans un menu) façon Serum : formes SIN / TRI / SAW / SQR / **S&H** (avec SLEW pour lisser les marches), aperçu de la forme, FREQUENCY, FADE-IN, RETRIG, synchro tempo Ableton.
- **4 macros** et **matrice de modulation à 8 slots** (sources : LFO 1-4, macros 1-4, env filtre, vélocité, molette ; destinations : pitch, pitch VCO2, PWM 1/2, FM, ring, mix VCO2, HPF, LPF, reso, level, pan).
- **Filtres MS20 retravaillés** : le HPF (cutoff plus haut, pic de résonance libre) gonfle les basses quand on monte la résonance vers 2 ; le LPF est plus sauvage (saturation asymétrique dans la boucle, la bande passante est ajoutée à la sortie : résonance criarde).
- **Wavefolder** (repli sinus, FOLD + SYMMETRY) placé avant les filtres ; destination de la matrice.
- **Arpégiateur** : UP / DOWN / UP-DN / RAND, division (1/4 à 1/32, avec triolets), 1 à 4 octaves, gate, ratchet x1-x4, chance, hold. Calé sur le tempo et la position d'Ableton.
- **Matrice** dans un volet déroulant (bouton MATRIX) visible sur les deux onglets ; les destinations incluent maintenant les effets (FX DRIVE, CLIP, UNCERT., HPF, LPF, PEAK, DESTROY, D.RATE, SPRING, DRY/WET, CHAOS) et le delay BBD (TIME, FEEDBACK, MIX). Les sources LFO sont alors les LFO libres (non retriggés).
- Molette de pitch (±2 demi-tons) et mod wheel (CC1).

## Effets (onglet EFFECTS / Orage FX)
Rack réordonnable, **delay BBD** (TIME, FEEDBACK, TONE, WOBBLE, MIX, PING-PONG, synchro tempo), **prises incrustées sur la face de chaque module** (câbles fins, bouton CABLES pour les masquer, survol d'une prise = câble en surbrillance), Gate, Octaver, Distorsion, Color, Filtre, Destroy, Tube Comp (ATTACK/THRESHOLD/VU), Spring, LFO/S&H/Envelope, presets, trois remises à zéro.

## Compilation (GitHub Actions)
1. Remplace dans ton dépôt `CMakeLists.txt` et le dossier `Source/` par ceux de ce zip.
2. **Remplace aussi le contenu de `.github/workflows/build.yml`** (il récupère maintenant les deux plugins).
3. Lance le workflow, télécharge l'artefact `Orage-VST3-Windows` : dossiers `Orage.vst3` et `Orage FX.vst3` à copier dans `C:\Program Files\Common Files\VST3`.
4. Dans Ableton : Orage se glisse sur une piste MIDI, Orage FX sur une piste audio.

## Design
Façade façon Korg MS20 : zones délimitées par des traits blancs sérigraphiés, potards numérotés 0-10, symboles de formes d'onde. Les fenêtres de confirmation (vider le rack, patchbay, reset, sauvegarde) sont intégrées au plugin. La fenêtre se redimensionne.
