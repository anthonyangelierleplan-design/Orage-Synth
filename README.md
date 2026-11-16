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

## v16 (audit du moteur)
- **Plus d'escalier sur les réglages** : les réglages continus du synthé et du rack FX sont lissés (courbe en S, ~20 ms) et les coefficients des filtres sont interpolés échantillon par échantillon.
- **Filtre MS-20** : la résonance ne laisse plus fuir les aigus quand le filtre est fermé (la bande ajoutée à la sortie est refiltrée).
- **Stabilité** : la FM extrême sur une note aiguë pouvait produire un signal invalide (NaN) ; les phases sont bornées et un garde-fou remet la voix à zéro en cas de valeur folle. Testé sur des centaines de réglages aléatoires extrêmes.
- **Anneaux** : le point vivant suit aussi les LFO ; réassigner une source dont la quantité était à zéro lui donne 50 %.

## v18 (filtre MS-10 modélisé d'après le circuit)
- **Nouveau type de filtre « MS-10 CIRCUIT »** (4ᵉ choix du sélecteur, les anciens filtres sont conservés et les états sauvegardés restent compatibles) : modèle du circuit Korg 35 du MS-10 (deux étages OTA-C, rétroaction par l'ampli à diodes 1N4148 et le potard PEAK), résolu à chaque échantillon (intégration trapézoïdale + Newton), sans tableau de réponse en fréquence.
- **Calibré sur de vraies mesures** d'un MS-10 de 1978 (résonance 0 / 30 / 60 / 100 % et auto-oscillation) : bosse de résonance, fréquence d'auto-oscillation (écart < 1 % dans le moteur), amplitude et harmoniques 2 et 3.
- **Limites connues** : seulement 3 positions de résonance mesurées (le comportement entre 60 % et 100 % est interpolé) ; l'affaiblissement des aigus au-delà de 7-9 kHz vu sur l'enregistrement n'est pas modélisé ; pas encore écouté ; crête max plus haute que le MS-20 à fond (1,74 contre 1,11 en pire cas, 5 voix) ; CPU environ 1,6× celui de l'ancien MS-20.

- **Nouveau « LADDER CEM3320 »** : modèle du circuit CEM3320 du Pro-One (4 étages OTA à tanh, retour implicite par le VCA de résonance), l'auto-oscillation suit le cutoff (< 1 %). **Nouveau « SEM CKT »** : modèle du circuit Oberheim SEM (mélangeur, deux intégrateurs CA3080, diodes D11/D12 sur le retour, morph LP > notch > HP conservé). Calibrés sur les schémas et la théorie du circuit, PAS sur des enregistrements du matériel (aucun disponible) : jugement à l'oreille. CPU 11,5 % et 10,8 % (6 voix) contre 7-9 % pour les anciens.

## v18d (stabilité des filtres circuit)
- **Cri strident du LADDER CEM3320 cutoff ouvert à fond : corrigé.** Cause : avec le cutoff très haut, le calcul du filtre (pas d'intégration trop grand pour la résolution de Newton) ne convergeait plus et partait en oscillation parasite jusqu'à pleine échelle, même à résonance moyenne.
- **Parasites / instabilité en polyphonie (SEM CKT surtout) : corrigé.** Deux causes : le même défaut de calcul sur les notes aiguës (le suivi de clavier pousse leur cutoff vers le haut), et une erreur de schéma dans mon SEM : les diodes réduisaient l'amortissement à fort niveau au lieu de l'augmenter, donc la résonance pouvait s'emballer.
- **Correction** : les filtres LADDER CEM3320 et SEM CKT tournent à cadence double (suréchantillonnage ×2), résolution amortie, topologie du SEM corrigée (amortissement fixe, régénération écrêtée par les diodes).
- **Mesure** : sur 2176 réglages (notes seules et accords, 44,1 et 48 kHz, cutoff jusqu'au maximum), 270 cas avaient des parasites au-dessus de -20 dB avant ; 0 après, au niveau des anciens filtres. Coût : CPU 15,7 % (Ladder CEM3320) et 13,6 % (SEM CKT) pour 6 voix, contre 11 % avant.
- Le MS-10 CKT auto-oscille fort (crête pleine échelle) à résonance 100 % et cutoff ouvert : c'est le comportement calibré sur le vrai MS-10, pas un défaut de calcul.

## v18c (anneaux, FX, presets)
- **Anneaux de modulation** : 100 % = tour complet du potard. Avant, 100 % n'agissait que sur la moitié de la course pour le filtre, la largeur d'impulsion et les EQ du rack, et sur 60 % pour les autres FX (l'anneau et le son étaient d'accord, mais pas avec le potard). La matrice compense maintenant l'échelle interne. Les réglages de modulation déjà sauvegardés dans tes anciens presets agiront donc plus fort (×2 sur le filtre).
- **FX** : quand on allume un module du rack avec son bouton, tous ses potards repartent de leur valeur neutre (plus de potard laissé à fond). Les presets d'usine du synthé démarrent aussi tous les FX à zéro ; seuls les 6 presets « Rack FX » gardent leurs anciens réglages de départ.
- **Refonte totale des presets du synthé : 74 presets** (Pad 14, Bass 15, Lead 14, Pluck / Keys 12, Arp / Seq 10, FX / Texture 9) écrits pour les filtres MS-10 CKT, LADDER CEM3320 et SEM CKT. Niveaux calibrés par mesure (RMS cible par catégorie, crête 0,4 pour les plucks et arpèges), 0 valeur invalide, CPU max 9 % d'un cœur. Non écoutés : à juger à l'oreille.

## v17 (qualité)
- **Suréchantillonnage ×4** du wavefolder et du drive, triangle anti-replié : beaucoup moins d'aigus parasites sur les notes hautes.
- **Coupe-continu** en sortie du synthé.
- Petite optimisation de la FM quand elle est à zéro.
- `KNOWLEDGE.md` : base de connaissances (architecture, journal des problèmes, règles, tests, checklist).

## v19 (adaptation à l'ASM Hydrasynth)
- **Carte CC par défaut** (table « Sorted by CC Number », p. 120 du manuel Hydrasynth), valable pour Orage et Orage FX, un CC = un seul paramètre : filtre (cutoff 74, résonance 71, drive 50, keytrack 51, quantité d'enveloppe 54), passe-haut sur Filter2 (55/56), glide 5, bruit 3, master 7, VCO1 vol 44, VCO2 vol 46, VCO2 cent 112 (accord fin), ring 43, FM 30, largeur d'impulsion 24/26, détune unison 95, LFO1-4 rate 72/73/76/78, ENV1 (81-84) = enveloppe de filtre, ENV2 (85-88) = enveloppe d'ampli, reverb mix 91 / decay 65 / tone 67, delay time 15 / feedback 14 / tone 63 / mix 92, arpégiateur : division 106, gate 107, mode 108, chance 110, octaves 120, longueur 122. Macros sur CC 16-23. Le MIDI learn (clic droit) redirige tout ; seuls les écarts sont sauvegardés.
- **Program Change** → preset d'usine (0 = Init), chargé sur le thread de l'interface.
- **Pédale de sustain** (CC64) : les fins de note sont retenues tant qu'elle est enfoncée (arpégiateur compris).
- **Étendue du pitch bend** réglable (±1 à ±24 demi-tons, défaut ±2), sélecteur dans le bloc MODULATION.
- Test : tools/tests/midi_hydrasynth.cpp (collisions, CC74, Program Change, sustain, bend).
- **Potard LFO 1 du filtre** (bloc VC LOWPASS FILTER, CC 52 comme « Filter1 LFO1amt ») : quantité de LFO1 sur le cutoff, 0 par défaut. Même échelle qu'un slot de matrice à 100 %.

## v19e (retours de test sur l'Hydrasynth)
- **Spring sur les CC de la reverb** : mix 91, decay 65, drip (ton) 67. La reverb passe en 93 (mix) et 94 (decay). L'Hydrasynth n'a pas de CC pour le spring, d'où ce choix.
- **Rack par défaut = delay + spring** (mix à zéro, démarrage neutre) pour une nouvelle instance et pour le preset Init.
- **Molette de modulation** : plus de routage par défaut vers le cutoff dans les presets d'usine (reste réglable dans la matrice).
- **Arpégiateur réorganisé comme la face avant de l'Hydrasynth** : TEMPO, CHANCE, GATE, SWING + DIVISION + RATCHET ; ON/LATCH, TRIPLET, TAP TEMPO ; MODE (direction) ; OCTAVE de 1 à 4 (l'ancien « FULL » = 4 octaves devient « 4 OCT »).
- **Ratchet global** 1x à 4x (CC 109), en plus du ratchet par pas ; **triolets** (pas x1,5) ; **tap tempo** (passe l'horloge en mode libre).
- Les boutons ON et LATCH de l'Hydrasynth n'envoient pas de CC : à régler à la main (ou par MIDI learn d'un autre contrôle).
- Correction : les 4 LFO du bloc MODULATION GENERATOR s'empilaient après un redimensionnement.
- **Potards crantés** (MODE, OCTAVE, RATCHET) : un cran par choix, graduations nommées autour du potard, valeur active en orange ; ON/OFF/LATCH, TRIPLET, TAP TEMPO, TYPE et FREE RUN restent des boutons. (`KnobUnit::setChoices` sur un paramètre de type choix.)

## v20 (qualité, lissage MIDI, aftertouch)
- **Filtres** : les trois filtres de première génération (MS-20, SEM, LADDER) sont supprimés. Il reste les trois modèles circuit : **MS-10**, **LADDER CEM3320**, **SEM**. Les anciens projets et presets sont migrés : MS-20 → MS-10, SEM → SEM, Ladder → CEM3320 (le timbre change légèrement).
- **Panneau SETTINGS** (bouton en haut à droite) : qualité audio, lissage MIDI, courbe et réactivité de l'aftertouch, mode de reprise MIDI. Ces réglages sont globaux (fichier de mapping MIDI + projet), pas dans les presets.
- **Qualité audio** : *Performance* (filtres ×2), *Quality* (×4, défaut), *High Fidelity* (×8 sur filtres, repliement et drive). Mesure sur 6 voix, un cœur : Performance 10–15 %, Quality 10–25 %, High Fidelity 14–50 % (CEM3320 le plus coûteux). Le MS-10 n'est pas suréchantillonné ; les oscillateurs restent en BLEP.
- **Lissage MIDI** : glissement des potards pilotés en MIDI (0 / 5 / 10 / 20 / 40 / 80 ms, défaut 20 ms) contre le bruit de zipper d'un contrôleur 7 bits.
- **Aftertouch** : courbe linéaire ou exponentielle, réactivité lente / moyenne / rapide.
- Tests : `tools/tests/midi_hydrasynth.cpp`, `stabilite_filtres.cpp` (nouvelle numérotation, argument qualité).

## v20b (vibrato VCO1, LFO rapides)
- **VCO 1** : trois nouveaux potards à côté de PW : **LFO AMT** (LFO 1 → hauteur du VCO 1), **+ WHEEL** et **+ AFTER** (profondeur ajoutée par la molette de modulation et par l'aftertouch). Profondeur quadratique : jusqu'à ±3 demi-tons, fin pour les petits vibratos. Mappables en MIDI learn.
- **LFO** : plage étendue à 0,05 – 300 Hz (avant 0,05 – 40 Hz). Les presets d'usine et les anciens projets sont convertis pour garder exactement leur vitesse (propriété d'état `lfoV2`). Le plafond est de 300 Hz car les LFO sont calculés par blocs de 16 échantillons.

## v20c (tiroirs de l'onglet Effets)
- Deux languettes dans l'en-tête de l'onglet Effets : **PATCHBAY** (colonne de droite) et **MODULATION** (bandeau LFO 1 / LFO 2 / S&H / suiveur d'enveloppe) se replient d'un clic ; le rack récupère la place. L'état est mémorisé (`drawers.txt`, dossier Foudre Audio). Replié, le câblage et les modulations restent actifs, seuls les câbles disparaissent de l'écran.

## v21 (MPE, circuits animés, presets nettoyés)
- **MPE** (SETTINGS → MPE MODE) : une note par canal (2–16, le canal 1 est le canal maître). Pression, slide (CC74) et pitch bend sont propres à chaque note ; le slide agit comme une molette par note (il alimente la source WHEEL et le potard + WHEEL du vibrato), la pression alimente l'aftertouch polyphonique. Plage de bend par note réglable (±2 à ±48, défaut MPE = 48, à aligner avec le clavier). Sur le Hydrasynth : mode MPE. Test : `tools/tests/midi_mpe.cpp`.
- **Faces animées** : chaque module du rack affiche une grille transparente avec des composants (résistance, condensateur, diode, circuit intégré, bobine, lampe) reliés par des pistes. Ils s'allument de gauche à droite avec le niveau de sortie du module, et des impulsions courent sur les pistes ; un module contourné reste éteint.
- **Presets** : à chaque chargement, les modules sans effet (mix à zéro, octaver éteint, filtre ouvert, etc.) sont retirés du rack (2,4 modules par preset en moyenne, contre 11). Un module reste s'il est câblé, modulé par le preset ou ciblé par un câble CV. Les macros qui visaient un module retiré sont redirigées vers un paramètre du synthé. Le module peut toujours être ajouté à la main.
- **Plucks / Keys** : decay (amp et filtre) ×1,8 et release ×1,8 (borné 0,35–0,75), pour des notes moins sèches.
