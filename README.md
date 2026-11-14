# Orage v7 — Foudre Audio

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

## v14
- **8 pistes de modulation** dans l'onglet ARP (pages LANES 1-4 / 5-8), chacune avec sa destination, sa longueur et son glide. Les pistes 5-8 sont aussi des sources de la matrice.
- **Matrice à 16 slots** (au lieu de 8), avec beaucoup plus de destinations : enveloppes ADSR amp et filtre, glide, key track, reverb (size, pre-delay, shimmer, width), BBD (tone, wobble), spring, comp, gate, octaver, Color (low/mid/high), arpégiateur (rate, gate, swing, chance), vitesse des 4 LFO.
- **Menus de destination/source en sous-menus** classés par module (OSCILLATORS, FILTER, ENVELOPES, LFO, ARPEGGIATOR, FX RACK > module…).
- **Molette de modulation (CC1)** : routage par défaut molette → LPF (slot 13 de la matrice) dans chaque preset ; modifiable.
- Anciens projets et presets restent compatibles.

## v15
- **Latence MIDI** : les contrôleurs MIDI sont appliqués au début du bloc audio (avant la lecture des réglages) et une seule fois par bloc : plus de bloc de retard, moins de messages vers l'hôte.
- **Drive du LPF plus brillant** : accentuation des médiums-aigus avant la saturation ; le drive n'assombrit plus et gonfle moins les graves.
- **Anneaux de modulation (2 sources par potard)** : clic droit sur un potard > « Assigner à » (LFO, macros, enveloppes, jeu, pistes arp) ; anneaux bleu et turquoise autour du potard avec un point blanc qui montre la valeur modulée en direct (macros, molette) ; Ctrl + clic gauche + glisser règle la quantité (zone extérieure du potard = anneau 1, centre = anneau 2). Utilise les slots de la matrice.
