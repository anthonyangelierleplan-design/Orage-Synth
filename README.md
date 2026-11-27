# Orage — Foudre Audio

Synthé analogique sauvage (filtres modélisés d'après les circuits) et rack d'effets à lampes. Deux plugins, même design, même dépôt :

- **Orage** (instrument VST3) : synthé 6 voix + rack d'effets. Onglets SYNTH / ARP / EFFECTS + volet MATRIX.
- **Orage FX** (effet VST3) : le rack d'effets seul, pour les pistes audio.

## Installer
Chaque push sur `main` publie `Orage-VST3-Windows.zip` dans la Release **latest** du dépôt.
Dézipper et copier `Orage.vst3` et `Orage FX.vst3` dans `C:\Program Files\Common Files\VST3` (remplacer l'ancienne version : mêmes identifiants, les projets se rouvrent).

## Synthé (onglet SYNTH, rangé selon le trajet du signal)
- **Sources** : 2 VCO SAW / PULSE / TRI / SIN, suréchantillonnés, modèle analogique ou numérique propre, phase libre ou recalée ; hard sync sans repliement, ring, bruit, glide ; vibrato du VCO 1 (LFO AMT, + WHEEL, + AFTER).
- **FM** : linéaire (façon DX) ou exponentielle, FEEDBACK, KEY SCALE, 18 rapports, 17 caractères (CHARACTER) et DICE.
- **Filtres circuit** : MS-10, LADDER CEM3320, SEM (morph LP / notch / HP) ; passe-haut, DRIVE, KEY TRACK, LFO 2, courbe de réponse en direct. Les trois filtres sonnent **au même volume** à réglage égal (chacun garde son timbre).
- **Enveloppes** ADSR filtre et ampli (LINK : l'enveloppe filtre pilote aussi le volume, l'enveloppe ampli devient ENV 2 libre), **4 LFO** de 0,05 à 1000 Hz, **wavefolder**.
- **Matrice 16 slots** avec un sens par routage (avant / arrière / les deux), anneaux de modulation sur les potards (clic droit > Assigner à), **8 macros**.
- **KEY ASSIGN** : POLY / MONO / UNISON / SHARE / CHORD, HOLD ; changer de mode ne coupe pas la note.

## Arpégiateur et séquenceur (onglet ARP)
- Un seul choix **OFF / ARP / SEQ** : ARP joue les touches tenues, SEQ joue les pas mémorisés (transposés par la touche jouée). Les réglages qui ne servent pas dans le mode choisi sont grisés.
- **HOLD** (les notes et le motif continuent quand on lâche les touches ; c'est aussi le HOLD de KEY ASSIGN), **RUN** (le séquenceur et les pistes tournent sans toucher le clavier), **THRU** (les touches jouées sonnent aussi).
- Horloge : **DIVISION** calée sur le tempo (dont 1/4T, 1/8T, 1/16T pour les triolets) ou **HZ (free)** : horloge libre réglée par le potard HZ (grisé sinon) ; TAP TEMPO ; swing, chance, gate.
- Grille de 16 pas (silence / note / accent), **ratchet** et **vitesse** par pas (×1, ×2, ×4, ×½, ×3/2) ; **ALL RATCHET** passe tous les pas au ratchet suivant.
- **Chord Seq** : en SEQ, REC puis jouer ; les notes jouées ensemble (moins de 60 ms d'écart) vont dans le même pas, jusqu'à 6 notes, et le pas affiche le nom de l'accord (Fm7, D♭maj7, C/E…). Bouton **CHORD / 1 NOTE** (1 NOTE = une note par pas, comme avant). Molette sur un pas = transpose l'accord entier.
- 7 modes d'arpège : UP, DOWN, UP-DN, PLAYED, RANDOM, CONVRG, **CHORD** (l'accord entier à chaque pas) ; 1 à 4 octaves.
- **SEQ** : enregistrement pas à pas, calé sur le DAW.
- **8 pistes de modulation** (destination, longueur, glide), aussi sources de la matrice.

## Effets (onglet EFFECTS / Orage FX)
- 12 modules, rack réordonnable, bypass par module : OCTAVER, DISTORSION (avec incertitude), COLOR, FILTER, DESTROY, COMP (à lampe), SPRING, GATE, BBD DELAY, REVERB (shimmer, freeze), STUTTER, CASSETTE / VHS.
- **PATCHBAY** (câbles audio et CV) **cachée par défaut** : sans câble, le rack joue les modules dans l'ordre. Le bouton **PATCH** (en haut à droite de l'onglet EFFECTS) la montre ; elle s'ouvre d'elle-même si le projet a des câbles (« PATCH · 3 câbles »).
- **MODULATION** (LFO 1, LFO 2, S&H, suiveur d'enveloppe) dans un tiroir repliable.
- Faces animées (circuits qui s'allument avec le niveau). Tous les modules démarrent neutres.

## MIDI et SETTINGS
- Carte CC de l'**Hydrasynth** par défaut, MIDI learn par clic droit, reprise SCALE / PICKUP / DIRECT, lissage MIDI, courbe et réactivité de l'aftertouch.
- **MPE** (pression, slide CC74 et bend par note), **profil Osmose CE** (SETTINGS > CONTROLLER).
- Program Change → preset d'usine, pédale de sustain (arpège compris), étendue du pitch bend réglable, macros sur CC 16 à 23.
- **Qualité audio** : Performance / Quality (défaut) / High Fidelity. Réglages globaux sauvés avec le projet, jamais dans les presets.

## Design
- Quatre thèmes : CHAMPAGNE, ARGENT, CUIVRE, ORIGINAL. Flancs en bois, anneaux lumineux, oscilloscope.
- Fenêtre redimensionnable, cran magnétique au zéro des potards, potards crantés pour les choix (MODE, OCTAVE, RATCHET…).
- 88 presets d'usine + Init (Orage FX : 7 presets d'effets + Init).

## Pour aller plus loin
- `CHANGELOG.md` : ce qui a changé à chaque version, avec les mesures.
- `KNOWLEDGE.md` : base de connaissances Foudre Audio (pièges, méthodes, checklist avant envoi).
- `tools/tests/LISEZMOI.md` : les tests ; `tools/presets/LISEZMOI.txt` : la génération des presets.

## Compilation
JUCE 8.0.4 (téléchargé par CMake), C++17. GitHub Actions : les tests automatiques d'abord, puis le build Windows ; la Release `latest` n'est publiée que depuis `main`.
En local : `cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release`.
