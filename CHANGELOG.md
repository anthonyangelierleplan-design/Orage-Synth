# Orage — historique des versions

Le plus récent en haut (v14 et suivantes ; les versions antérieures ne sont pas documentées). L'état actuel du plugin est décrit dans le README.

## v26 (ergonomie : séquenceur, en-tête, patchbay, LFO animés, arpèges plus tenus)
- **Séquenceur** : sélection de pas (clic ou glisser sur les numéros, MAJ + clic, ÉCHAP), **CTRL+D** duplique la sélection juste après elle (sans sélection : tout le motif, longueur doublée), **CTRL+Z / CTRL+Y** annuler / refaire (64 retours : pas, notes, accords, pistes, RANDOM, CLEAR, ALL RATCHET, REST / TIE / BACK, molette, longueur). Les notes enregistrées au clavier en REC ne passent pas par l'historique.
- **Grille NOTE STEPS** : numéros lisibles au-dessus des cases, groupes de 4 pas (pistes alignées), accent = case plus lumineuse (plus de barre blanche), vitesses et badges plus grands, tête de lecture = cadre blanc.
- **Écran des LFO animé** : l'onde défile comme sur un oscilloscope à la vitesse réelle du LFO (bord droit = maintenant, point = valeur actuelle) ; en RETRIG, suit la note la plus récente. 60 images/s, seulement quand l'écran est visible. Le son ne change pas.
- **En-tête** : MIDI, LOOK, SETTINGS et le moniteur MIDI passent dans la barre des onglets ; l'en-tête garde logo, lampe et preset, aligné sur la colonne de la patchbay ; CABLES / PATCH / MODULATION en deuxième ligne (onglet EFFECTS seulement). Orage FX : LOOK reste dans l'en-tête.
- **Patchbay** : MODULATION OUT et CV IN suivent juste sous la liste audio (avant : place réservée pour 13 modules, grand vide) ; entrées CV des modules présents serrées, celles des modules absents en liste grisée.
- **ON / OFF d'un module** : vraie comparaison A/B, les réglages sont conservés (avant : les potards repartaient à leur valeur par défaut au rallumage). Un module **ajouté** au rack repart de ses réglages neutres.
- **Arpèges moins « pluck »** : Arp Rain, Pulse Climb, Random Dreams, Chord Pulse 82, Half-Speed Memory et Pine Forest Signal ont des notes plus tenues (sustain ≥ 0,6, relâche ≥ 0,45, sustain du filtre ≥ 0,25, gate ≥ 75 %), au **même volume ressenti** qu'avant (corrections -3,8 à -11,1 dB, mesurées au banc). Seq Pluck, Ratchet Pluck, Acid Lanes, Trance Gate Arp, Bass Sequence et Stutter Sequencer restent pincés exprès. Pine Forest Signal : crête -7,8 dBFS (avant -0,8).
- Les anciens projets se rouvrent à l'identique (rien de sauvé n'a changé de format). Banc : `FOUDRE_PATCH=1` ouvre la patchbay pour les captures.

## v25 (module TAPE, calé sur un Philips D6410)
- **Module TAPE** à la place de CASSETTE / VHS (même emplacement du rack, pas de module en plus) : un magnétophone à cassette calé sur un Philips D6410 mesuré au banc (fichier témoin enregistré sur cassette type I puis réenregistré). Code commun avec Zephyr dans Foudre-Core (`FoudreTape.h`).
  - DRIVE : saturation de bande suréchantillonnée ×4 (harmonique 3 surtout) ; 50 % = passe propre du Philips (H3 -24 dB, 6 à 9 % de distorsion), 100 % = passe saturée (12 à 17 %).
  - ALC : compression automatique d'enregistrement du Philips (attaque 60 ms, relâche 0,7 s, jusqu'à 5:1 : 19 dB d'écart à l'entrée → 4 dB à la sortie).
  - COLOR : courbe mesurée (bruit rose) : -10 dB à 160 Hz, -24 dB à 63 Hz, +10 dB vers 8 kHz, coupure au-delà de 12 kHz (écart au Philips : 1,3 dB rms de 40 Hz à 20 kHz).
  - WOW / FLUTTER : raies mesurées (1,4 Hz ; 6,3 - 7,2 - 9,4 Hz ; 72 Hz) + dérive aléatoire ; 50 % = Philips (0,10 % / 0,21 % rms), jusqu'à 8× / 3× au-delà.
  - HISS : souffle à la couleur du Philips, 100 % = -40,7 dBFS ; loi au carré (50 % = -52,7 dBFS). HUM : 50 Hz et harmoniques, 10 % = Philips.
  - MONO (le Philips est mono), MIX. Tout à zéro : sortie = entrée, retardée de 36 échantillons (0,75 ms, le son direct de MIX attend autant : pas d'effet de peigne).
  - Vérifié : le fichier témoin passé dans TAPE réglé « Philips » donne les mêmes mesures que la vraie cassette (distorsion, courbe, pleurage, souffle, ronflement).
- **Anciens projets** : le module reste l'ancien CASSETTE (même son), titre « CASSETTE / VHS (old project) » et bouton **→ TAPE** pour passer au nouveau moteur.
- **Presets** : les presets cassette passent sur TAPE (volumes recalés sur ceux d'avant) ; **6 presets façon Boards of Canada** : Kodachrome Fade (Pad), Summer Camp 1979 (Lead), Pine Forest Signal (Arp), Overwound Spool (Bass), Film Society (Chord Seq : Fm7, D♭maj7, A♭, E♭), Rewind Memory (Rack FX, Philips complet).

## v24 (Chord Seq, Arp/Seq rangé, patchbay cachée, filtres au même volume)
- **Chord Seq** : en SEQ, les notes jouées ensemble (moins de 60 ms d'écart) vont dans le même pas, jusqu'à 6 notes ; le pas affiche le nom de l'accord (Fm7, D♭maj7, C/E…) et sa basse. Bouton CHORD / 1 NOTE ; molette = transpose l'accord. Le nom des accords et le regroupement des notes viennent de Foudre-Core (`FoudreAccords.h`, commun avec Zephyr).
- **Arp/Seq rangé** (doublons retirés) : un seul choix OFF / ARP / SEQ (avant : OFF-ON-LATCH puis ARP-SEQ) ; LATCH fusionné avec HOLD ; TRIPLET retiré (les triolets sont dans DIVISION) ; FREE renommé RUN ; « Free » de DIVISION renommé HZ (free) et le potard TEMPO renommé HZ, grisé quand l'horloge suit le tempo ; potard RATCHET global remplacé par ALL RATCHET sur la grille ; réglages inutiles grisés selon le mode (MODE / OCTAVE en SEQ, REC / REST / TIE / BACK / CHORD en ARP).
- **Anciens projets convertis à l'identique** : LATCH → ARP + HOLD ; TRIPLET + 1/4, 1/8, 1/16 → 1/4T, 1/8T, 1/16T ; TRIPLET en horloge libre → HZ ×1,5 ; RATCHET global → chaque pas de la grille (le moteur gardait déjà le plus grand des deux). Mesure : projet converti et non converti identiques (0,0 dB). TRIPLET reste affiché dans le seul cas sans équivalent (triolet sur une division en mesures ou pointée).
- **Patchbay cachée par défaut** : bouton PATCH ; s'ouvre d'elle-même si le projet a des câbles, avec leur nombre sur le bouton. Le son ne change pas (sans câble, le rack jouait déjà dans l'ordre).
- **Filtres au même volume** (en gardant leur caractère) : à cutoff et résonance égaux, MS-10, LADDER et SEM sont à ±0,2 dB l'un de l'autre sur la note de mesure, ±2 dB sur une note deux octaves plus haut (avant : jusqu'à 4,7 dB d'écart). Correction mesurée au banc d'essai (table selon la position du cutoff par rapport à la note et la résonance) ; pas de correction filtre fermé sous la note. Les **anciens projets gardent l'ancien volume** ; les **presets d'usine** sont recalés à leur volume d'avant (60 presets, ±0,3 dB) ; le son Init baisse de 0,7 dB.
- Tests : banc d'essai Foudre-Core (enregistrement d'accords, conversion d'anciens projets, niveaux des filtres et des presets), `tools/tests/accords.cpp` dans Foudre-Core.

## v23 (cassette, arpège en accords, vitesse par pas)
- **Module CASSETTE / VHS** (rack d'effets, 12e module) pour un son qui a l'air vieux, façon Boards of Canada. Ordre interne : vieil échantillonneur → enregistrement saturé → lecture qui pleure → bande usée → souffle.
  - **WOW** : la hauteur flotte lentement (jusqu'à ~40 cents crête à crête, avec une dérive irrégulière du moteur). **FLUTTER** : tremblement rapide de la bande.
  - **SATURATE** : saturation de bande suréchantillonnée ×4 (gain unité à bas niveau, un peu de compression). **AGE** : les aigus partent (19,5 kHz → 2,4 kHz), bosse de tête de lecture, petites pertes de niveau d'une bande usée au-delà de la moitié.
  - **HISS** : souffle de bande. **CRUNCH** : vieil échantillonneur (fréquence 32 → 5,5 kHz, résolution 15 → 6,5 bits). **MIX**.
  - Tout à zéro = transparent (aucun retard tant que WOW et FLUTTER sont à zéro). Placé en fin de chaîne par défaut (il « enregistre » tout le reste).
  - Pas encore de destination de matrice pour ses potards.
- **Arpégiateur : mode CHORD** (7e cran du potard MODE) : à chaque pas, l'accord tenu (ou l'accord mémorisé en KEY ASSIGN CHORD) est rejoué en bloc. OCTAVE fait monter l'accord d'une octave à chaque pas joué. Accents, silences, ratchets, gate et swing s'appliquent à l'accord entier.
- **Vitesse par pas** : bouton **STEP SPEED** en haut à droite de NOTE STEPS (à côté de RATCHET) : la rangée du bas affiche la vitesse de chaque pas. Clic = vitesse suivante (×1 → ×2 → ×4 → ×½ → ×3/2), clic droit = ×1, glisser = même vitesse sur les pas voisins, molette = vitesse plus lente / plus rapide. Un pas ×2 dure deux fois moins longtemps (le motif avance plus vite), ×½ deux fois plus longtemps. En vue RATCHET, un petit badge rappelle la vitesse des pas modifiés. Marche en ARP comme en SEQ ; en SEQ calé sur le DAW, la position suit le morceau avec les vitesses.
- **Potards crantés** : les libellés latéraux ne sont plus coupés (le potard laisse la place aux textes les plus longs).
- **8 presets** : Faded Polaroid (Pad), Filmstrip Keys, Lullaby Sine (Lead), VHS Bass, Chord Pulse 82 (CHORD en 1/16, tenir un accord), Half-Speed Memory (SEQ avec pas ×2 et ×½), Rainy Projector (Texture), et Cassette Memories (Rack FX, aussi dans Orage FX). Niveaux calés sur ceux de leur catégorie.
- Compatibilité : anciens projets relus à l'identique (patch v4 → v5, motif sans vitesse = ×1). Le potard MODE a un cran de plus : sur l'Hydrasynth (CC 108), la course est répartie sur 7 positions au lieu de 6.
- Tests : `tools/tests/v23_cassette_arp.cpp` (sans JUCE), `tools/tests/v23_niveaux_presets.cpp`.

## v22f (profil de contrôleur : Hydrasynth / Osmose CE)
- **SETTINGS → CONTROLLER** : *Hydrasynth* (table MIDI d'origine) ou *Osmose CE*. Le profil Osmose active le MPE (canal 1 maître, une note par canal), règle le bend par note sur ±48 (valeur MPE d'origine de l'Osmose), vide la table MIDI (l'Osmose n'a pas de potards) et passe la reprise en DIRECT (slider et pédales sont absolus). Le slider de modulation (CC1) pilote déjà la molette ; la pression par note, le CC74 (axe aftertouch de l'Osmose) et le bend par touche sont gérés par le MPE existant. Pédales : clic droit sur un potard > Assign MIDI. Le choix est sauvé avec les réglages globaux. « RESTORE HYDRASYNTH MAP » revient au profil Hydrasynth.

## v22e (audit enveloppes et effets)
- **Plus de clic quand une voix est reutilisee** (vol de voix a la 13e note, MONO qui rejoue pendant le release) : la voix garde son etat comme sur un vrai synthe. Mesure : saut de +23 dB (vol) et +39 dB (MONO) -> au niveau du jeu normal.
- **Reverb en interpolation cubique** : plus propre (-52 -> -58 dB), meme couleur (amortissement et filtre de sortie recales, moins d'1 dB d'ecart).
- **BBD** : le souffle d'horloge suit maintenant le compresseur-expandeur d'un vrai BBD : present avec le signal, il s'efface dans le silence (-63 -> -93 dB).
- **Surechantillonneur elliptique** (-95 dB) pour la distorsion. Limite connue : une fuzz tres poussee sur des notes tres aigues (vers 3 kHz) replie encore un peu ; aux hauteurs habituelles c'est propre (-68 a -100 dB).
- Verifies sans defaut : enveloppes, crepitement des reglages, denormaux, bruit residuel. L'instabilite de la distorsion (UNC) est voulue.
- Tests : `tools/tests/continuite_voix`, `tools/tests/audit_effets_orage`.

## v22e (VCO)
- **VCO surechantillonnes** (x2, x4 en High Fidelity) : dent de scie, carre, FM, retour et sync restent propres jusque dans l'aigu (pire raie parasite audible : -47 -> -66 dB en dent de scie au Sol 7 ; FM lineaire -15 -> -51 dB). Cout processeur : +1 a +4 %.
- **Hard sync anti-repliement** : correction a l'instant exact du recalage (et le VCO 2 n'avance plus d'un echantillon de trop apres le recalage) : -24 -> -61 dB dans l'aigu, toutes formes d'onde.
- **SETTINGS > VCO PHASE** : *Free running* (defaut, comme un vrai VCO : chaque note demarre ailleurs dans le cycle) ou *Reset* (attaque identique a chaque note).
- **SETTINGS > VCO MODEL** : *Analog* (defaut : rampe legerement bombee, sommets du triangle arrondis, liaison capacitive ~5 Hz, petites differences entre les voix) ou *Clean digital*.
- Test : `tools/tests/vco_phase_sync`.

## v22d (corrections)
- **FM** : avec un rapport FM (1:2, 2:1, 3:1...), OCTAVE, TUNE et FINE du VCO 2 se reglent toujours, par-dessus le rapport (avant, ils n'avaient plus d'effet). CHARACTER et DICE remettent le VCO 2 au neutre pour que leur rapport s'entende tel quel.
- **Sequenceur sans arpegiateur** : TYPE = SEQ suffit, plus besoin d'allumer l'ARP (les touches transposent le motif ; LATCH reste disponible).
- **Sequenceur en tempo** : changer de note ne relance plus l'horloge ; elle continue meme entre deux notes. Quand le DAW joue (cadence synchronisee), les pas sont cales sur sa mesure. Une touche a peine en retard (< 30 ms) fait quand meme partir le pas en cours ; apres une longue pause (un motif entier), le motif repart du debut.
- Anciens projets : memes hauteurs et meme jeu (VCO 2 remis au neutre s'il etait desaccorde sans effet ; SEQ choisi avec ARP eteint -> type ARP).
- **LINK des enveloppes** (bouton sur le panneau AMP) : l'enveloppe FILTRE pilote aussi le volume ; l'enveloppe AMP devient **ENV 2 libre** (ses ADSR restent reglables) : source ENV AMP de la matrice, ou raccourci DEST (VCO 2 pitch, VCO 1+2 pitch, FM, PWM, wavefold, niveau VCO 2, ring) + AMOUNT (+-1 = +-1 octave pour la hauteur). LINK eteint : rien ne change.
- Tests : `tools/tests/seq_tempo_fm`, `tools/tests/env_link` ; `midi_hydrasynth` recompile de nouveau (acces aux membres prives retabli).

## v22c (gate recalé, LFO jusqu'à 1 kHz)
- **GATE de l'ARP (CC 107)** : l'Hydrasynth n'envoie que de 5 à 100 sur ce potard ; Orage recale cette plage sur 0–100 %. Les autres CC restent sur 0–127.
- **LFO 0,05 – 1000 Hz** (avant 300 Hz) pour les 4 LFO du synthé. Les presets et les projets existants sont convertis (propriété d'état `lfoV3`) : même vitesse qu'avant.
- **Précision** : quand un LFO utilisé (routé dans la matrice, vibrato, filtre) dépasse 160 / 350 / 700 Hz, le bloc de contrôle passe de 16 à 8 / 4 / 2 échantillons. À 1 kHz, la modulation reste une vraie onde (test : un LFO 1 kHz sur la hauteur crée bien les bandes latérales à f0 ± 1 kHz, `tools/tests/reprise_midi_chord.cpp`). Les LFO lents ne coûtent rien de plus ; le coût en CPU n'augmente que lorsqu'un LFO rapide est réellement utilisé.

## v22b (retours de test)
- **Drive du Ladder** : le DRIVE compense maintenant la perte de graves due au PEAK (comme le contour d'un Moog) : gain de sortie 1 + PEAK × (1,2 + 2,3 × drive). La courbe du filtre montre cette compensation et l'accent des aigus que le drive ajoute avant le filtre (tous modèles).
- **Reprise MIDI SCALE** : à chaque demi-tour du potard, la correspondance repart du point courant. Les butées (0 et 100 %) restent donc toujours atteignables, avant elles pouvaient rester à 90 % si le potard de l'Hydrasynth était déjà en butée. Test : `tools/tests/reprise_midi_chord.cpp` (PICKUP et DIRECT inchangés, vérifiés).
- **Potard LFO du bloc filtre** : il pilote maintenant **LFO 2** (LFO 1 est déjà sur la hauteur du VCO 1). Potard renommé « LFO 2 ».
- **CHORD LEARN** : les touches jouées pendant l'apprentissage sonnent (notes simples). L'accord appris est l'ensemble des notes tenues ensemble ; la note la plus basse est la fondamentale (plus de dépendance à l'ordre des doigts). L'apprentissage s'arrête tout seul au relâchement de toutes les touches (2 notes minimum).
- **Tuiles MODULES** : elles suivent l'ordre du rack (modules du rack d'abord, dans l'ordre où ils sont placés, puis les autres) et se réordonnent quand on déplace un module.
- Titre **WAVEFOLDER** complet (module élargi).

## v22 (nouvel agencement de la page SYNTH, matrice orientée, modes de jeu sans coupure)
- **Page SYNTH refaite selon le flux du signal** : ① Sources (VCO 1, VCO 2, MIXER) → ② VC FILTER (au centre : choix du modèle, courbe de réponse en direct, CUTOFF / PEAK / EG AMOUNT / DRIVE en grand, KEY TRACK, LFO 1, SEM MORPH, passe-haut) → ③ Enveloppes (filtre et ampli avec courbes ADSR en direct) et LFO / S&H. Bandeau du bas : FM, WAVEFOLDE, MACROS, KEY ASSIGN (avec DETUNE, PORTAMENTO, plage du bend) et **raccourci ARP** (OFF / ON / LATCH, RATE, GATE). SYNC et RING sont passés dans VCO 2.
- **Habillage** : flancs en bois à vis laiton, aération en haut à droite, lunettes métal autour des écrans (courbes), onglets décalés. Plus de chiffres 0–10 autour des potards : un anneau lumineux fin avec ses repères.
- **PATCHBAY / MODULATION** : les deux languettes n'apparaissent que sur l'onglet EFFECTS.
- **Modes de jeu sans coupure** : changer Poly / Mono / Unison / Share / Chord (ou HOLD) avec une note tenue ne la coupe plus ; le son continue dans le nouveau mode (`remapAssign`). Seul le changement d'ARP ou de mode avec l'arpégiateur actif relâche les notes.
- **Matrice : sens par routage** (FORWARD / BACKWARD / BOTH, colonne DIRECTION du panneau MATRIX et clic droit sur un potard → « Sens de l'anneau »). La quantité devient 0..1 et le sens est un réglage à part (`s_slotNDir`). L'anneau suit le sens et le son aussi : « avant » part vers le haut, « arrière » vers le bas, « les deux » s'ouvre des deux côtés du point du potard. Les anciens projets et presets sont migrés sans changer le son (propriété d'état `dirV3`).
- **SETTINGS → MIDI ASSIGNMENTS** : *CLEAR ALL ASSIGNMENTS* efface toutes les assignations MIDI (pour un autre contrôleur) ; *RESTORE HYDRASYNTH MAP* remet la table d'origine, qui reste la valeur par défaut.
- Test : `tools/tests/modes_matrice.cpp`.

## v21b (FM plus musicale, oscilloscope réglable)
- **Onde SINUS** sur les deux VCO (4ᵉ choix, après Saw / Pulse / Triangle) : indispensable pour une FM propre.
- **FM linéaire (modulation de phase, façon DX)** : sélecteur FM TYPE dans la page avancée. La FM « exponentielle » d'origine reste (EXP, défaut, les presets existants sont inchangés) mais elle fait dériver la hauteur : mesure à 440 Hz, rapport 2:1, profondeur 0,4 → fréquence moyenne 494 Hz en EXP (+200 cents), 439 Hz en LINEAR. Test : `tools/tests/fm_lineaire.cpp`.
- **FEEDBACK** (retour du modulateur sur lui-même, timbres riches sans stridence) et **KEY SCALE** (profondeur réduite dans l'aigu : sur la note 96, centroïde 6,1 kHz → 4,0 kHz).
- **18 rapports** (ajout de 5:2, 9:2, 6:1, 8:1, ×2.76, ×5.4 pour cloches et gongs) sur un potard RATIO à crans.
- **Panneau FM** : page principale (CHARACTER, DICE, AMOUNT, RATIO, ENV AMT, DECAY, ATTACK, VELOCITY, AFTERTOUCH) et page avancée (MORE : direction, modulateur, type, feedback, key scale, fine).
- **CHARACTER** (17) : Bell, E-Piano, Tine, Marimba, Glass, Brass, Organ, Gong, Wood, String, FM Bass, Clav, Choir, Space, Laser, Metal, Growl. Un clic règle ondes, ratio, indice, enveloppes FM et d'amplitude ; le modulateur reste muet (on entend la porteuse seule).
- **DICE** : tire un son FM musical (rapports harmoniques ou cloche, decay cohérent, modulation de phase).
- **Oscilloscope** : gain auto étendu à +52 dB, boutons − / + / AUTO (pas de 3 dB, Maj : 0,5 dB), molette (1 dB, Maj : 0,25 dB), double-clic sur AUTO : remise à zéro. Réglages mémorisés (`scope.txt`).

## v21 (MPE, circuits animés, presets nettoyés)
- **MPE** (SETTINGS → MPE MODE) : une note par canal (2–16, le canal 1 est le canal maître). Pression, slide (CC74) et pitch bend sont propres à chaque note ; le slide agit comme une molette par note (il alimente la source WHEEL et le potard + WHEEL du vibrato), la pression alimente l'aftertouch polyphonique. Plage de bend par note réglable (±2 à ±48, défaut MPE = 48, à aligner avec le clavier). Sur le Hydrasynth : mode MPE. Test : `tools/tests/midi_mpe.cpp`.
- **Faces animées** : chaque module du rack affiche une grille transparente avec des composants (résistance, condensateur, diode, circuit intégré, bobine, lampe) reliés par des pistes. Ils s'allument de gauche à droite avec le niveau de sortie du module, et des impulsions courent sur les pistes ; un module contourné reste éteint.
- **Presets** : à chaque chargement, les modules sans effet (mix à zéro, octaver éteint, filtre ouvert, etc.) sont retirés du rack (2,4 modules par preset en moyenne, contre 11). Un module reste s'il est câblé, modulé par le preset ou ciblé par un câble CV. Les macros qui visaient un module retiré sont redirigées vers un paramètre du synthé. Le module peut toujours être ajouté à la main.
- **Plucks / Keys** : decay (amp et filtre) ×1,8 et release ×1,8 (borné 0,35–0,75), pour des notes moins sèches.

## v20c (tiroirs de l'onglet Effets)
- Deux languettes dans l'en-tête de l'onglet Effets : **PATCHBAY** (colonne de droite) et **MODULATION** (bandeau LFO 1 / LFO 2 / S&H / suiveur d'enveloppe) se replient d'un clic ; le rack récupère la place. L'état est mémorisé (`drawers.txt`, dossier Foudre Audio). Replié, le câblage et les modulations restent actifs, seuls les câbles disparaissent de l'écran.

## v20b (vibrato VCO1, LFO rapides)
- **VCO 1** : trois nouveaux potards à côté de PW : **LFO AMT** (LFO 1 → hauteur du VCO 1), **+ WHEEL** et **+ AFTER** (profondeur ajoutée par la molette de modulation et par l'aftertouch). Profondeur quadratique : jusqu'à ±3 demi-tons, fin pour les petits vibratos. Mappables en MIDI learn.
- **LFO** : plage étendue à 0,05 – 300 Hz (avant 0,05 – 40 Hz). Les presets d'usine et les anciens projets sont convertis pour garder exactement leur vitesse (propriété d'état `lfoV2`). Le plafond est de 300 Hz car les LFO sont calculés par blocs de 16 échantillons.

## v20 (qualité, lissage MIDI, aftertouch)
- **Filtres** : les trois filtres de première génération (MS-20, SEM, LADDER) sont supprimés. Il reste les trois modèles circuit : **MS-10**, **LADDER CEM3320**, **SEM**. Les anciens projets et presets sont migrés : MS-20 → MS-10, SEM → SEM, Ladder → CEM3320 (le timbre change légèrement).
- **Panneau SETTINGS** (bouton en haut à droite) : qualité audio, lissage MIDI, courbe et réactivité de l'aftertouch, mode de reprise MIDI. Ces réglages sont globaux (fichier de mapping MIDI + projet), pas dans les presets.
- **Qualité audio** : *Performance* (filtres ×2), *Quality* (×4, défaut), *High Fidelity* (×8 sur filtres, repliement et drive). Mesure sur 6 voix, un cœur : Performance 10–15 %, Quality 10–25 %, High Fidelity 14–50 % (CEM3320 le plus coûteux). Le MS-10 n'est pas suréchantillonné ; les oscillateurs restent en BLEP.
- **Lissage MIDI** : glissement des potards pilotés en MIDI (0 / 5 / 10 / 20 / 40 / 80 ms, défaut 20 ms) contre le bruit de zipper d'un contrôleur 7 bits.
- **Aftertouch** : courbe linéaire ou exponentielle, réactivité lente / moyenne / rapide.
- Tests : `tools/tests/midi_hydrasynth.cpp`, `stabilite_filtres.cpp` (nouvelle numérotation, argument qualité).

## v19e (retours de test sur l'Hydrasynth)
- **Spring sur les CC de la reverb** : mix 91, decay 65, drip (ton) 67. La reverb passe en 93 (mix) et 94 (decay). L'Hydrasynth n'a pas de CC pour le spring, d'où ce choix.
- **Rack par défaut = delay + spring** (mix à zéro, démarrage neutre) pour une nouvelle instance et pour le preset Init.
- **Molette de modulation** : plus de routage par défaut vers le cutoff dans les presets d'usine (reste réglable dans la matrice).
- **Arpégiateur réorganisé comme la face avant de l'Hydrasynth** : TEMPO, CHANCE, GATE, SWING + DIVISION + RATCHET ; ON/LATCH, TRIPLET, TAP TEMPO ; MODE (direction) ; OCTAVE de 1 à 4 (l'ancien « FULL » = 4 octaves devient « 4 OCT »).
- **Ratchet global** 1x à 4x (CC 109), en plus du ratchet par pas ; **triolets** (pas x1,5) ; **tap tempo** (passe l'horloge en mode libre).
- Les boutons ON et LATCH de l'Hydrasynth n'envoient pas de CC : à régler à la main (ou par MIDI learn d'un autre contrôle).
- Correction : les 4 LFO du bloc MODULATION GENERATOR s'empilaient après un redimensionnement.
- **Potards crantés** (MODE, OCTAVE, RATCHET) : un cran par choix, graduations nommées autour du potard, valeur active en orange ; ON/OFF/LATCH, TRIPLET, TAP TEMPO, TYPE et FREE RUN restent des boutons. (`KnobUnit::setChoices` sur un paramètre de type choix.)

## v19 (adaptation à l'ASM Hydrasynth)
- **Carte CC par défaut** (table « Sorted by CC Number », p. 120 du manuel Hydrasynth), valable pour Orage et Orage FX, un CC = un seul paramètre : filtre (cutoff 74, résonance 71, drive 50, keytrack 51, quantité d'enveloppe 54), passe-haut sur Filter2 (55/56), glide 5, bruit 3, master 7, VCO1 vol 44, VCO2 vol 46, VCO2 cent 112 (accord fin), ring 43, FM 30, largeur d'impulsion 24/26, détune unison 95, LFO1-4 rate 72/73/76/78, ENV1 (81-84) = enveloppe de filtre, ENV2 (85-88) = enveloppe d'ampli, reverb mix 91 / decay 65 / tone 67, delay time 15 / feedback 14 / tone 63 / mix 92, arpégiateur : division 106, gate 107, mode 108, chance 110, octaves 120, longueur 122. Macros sur CC 16-23. Le MIDI learn (clic droit) redirige tout ; seuls les écarts sont sauvegardés.
- **Program Change** → preset d'usine (0 = Init), chargé sur le thread de l'interface.
- **Pédale de sustain** (CC64) : les fins de note sont retenues tant qu'elle est enfoncée (arpégiateur compris).
- **Étendue du pitch bend** réglable (±1 à ±24 demi-tons, défaut ±2), sélecteur dans le bloc MODULATION.
- Test : tools/tests/midi_hydrasynth.cpp (collisions, CC74, Program Change, sustain, bend).
- **Potard LFO 1 du filtre** (bloc VC LOWPASS FILTER, CC 52 comme « Filter1 LFO1amt ») : quantité de LFO1 sur le cutoff, 0 par défaut. Même échelle qu'un slot de matrice à 100 %.

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

## v18 (filtre MS-10 modélisé d'après le circuit)
- **Nouveau type de filtre « MS-10 CIRCUIT »** (4ᵉ choix du sélecteur, les anciens filtres sont conservés et les états sauvegardés restent compatibles) : modèle du circuit Korg 35 du MS-10 (deux étages OTA-C, rétroaction par l'ampli à diodes 1N4148 et le potard PEAK), résolu à chaque échantillon (intégration trapézoïdale + Newton), sans tableau de réponse en fréquence.
- **Calibré sur de vraies mesures** d'un MS-10 de 1978 (résonance 0 / 30 / 60 / 100 % et auto-oscillation) : bosse de résonance, fréquence d'auto-oscillation (écart < 1 % dans le moteur), amplitude et harmoniques 2 et 3.
- **Limites connues** : seulement 3 positions de résonance mesurées (le comportement entre 60 % et 100 % est interpolé) ; l'affaiblissement des aigus au-delà de 7-9 kHz vu sur l'enregistrement n'est pas modélisé ; pas encore écouté ; crête max plus haute que le MS-20 à fond (1,74 contre 1,11 en pire cas, 5 voix) ; CPU environ 1,6× celui de l'ancien MS-20.

- **Nouveau « LADDER CEM3320 »** : modèle du circuit CEM3320 du Pro-One (4 étages OTA à tanh, retour implicite par le VCA de résonance), l'auto-oscillation suit le cutoff (< 1 %). **Nouveau « SEM CKT »** : modèle du circuit Oberheim SEM (mélangeur, deux intégrateurs CA3080, diodes D11/D12 sur le retour, morph LP > notch > HP conservé). Calibrés sur les schémas et la théorie du circuit, PAS sur des enregistrements du matériel (aucun disponible) : jugement à l'oreille. CPU 11,5 % et 10,8 % (6 voix) contre 7-9 % pour les anciens.

## v17 (qualité)
- **Suréchantillonnage ×4** du wavefolder et du drive, triangle anti-replié : beaucoup moins d'aigus parasites sur les notes hautes.
- **Coupe-continu** en sortie du synthé.
- Petite optimisation de la FM quand elle est à zéro.
- `KNOWLEDGE.md` : base de connaissances (architecture, journal des problèmes, règles, tests, checklist).

## v16 (audit du moteur)
- **Plus d'escalier sur les réglages** : les réglages continus du synthé et du rack FX sont lissés (courbe en S, ~20 ms) et les coefficients des filtres sont interpolés échantillon par échantillon.
- **Filtre MS-20** : la résonance ne laisse plus fuir les aigus quand le filtre est fermé (la bande ajoutée à la sortie est refiltrée).
- **Stabilité** : la FM extrême sur une note aiguë pouvait produire un signal invalide (NaN) ; les phases sont bornées et un garde-fou remet la voix à zéro en cas de valeur folle. Testé sur des centaines de réglages aléatoires extrêmes.
- **Anneaux** : le point vivant suit aussi les LFO ; réassigner une source dont la quantité était à zéro lui donne 50 %.

## v15
- **Latence MIDI** : les contrôleurs MIDI sont appliqués au début du bloc audio (avant la lecture des réglages) et une seule fois par bloc : plus de bloc de retard, moins de messages vers l'hôte.
- **Drive du LPF plus brillant** : accentuation des médiums-aigus avant la saturation ; le drive n'assombrit plus et gonfle moins les graves.
- **Anneaux de modulation (2 sources par potard)** : clic droit sur un potard > « Assigner à » (LFO, macros, enveloppes, jeu, pistes arp) ; anneaux bleu et turquoise autour du potard avec un point blanc qui montre la valeur modulée en direct (macros, molette) ; Ctrl + clic gauche + glisser règle la quantité (zone extérieure du potard = anneau 1, centre = anneau 2). Utilise les slots de la matrice.

## v14
- **8 pistes de modulation** dans l'onglet ARP (pages LANES 1-4 / 5-8), chacune avec sa destination, sa longueur et son glide. Les pistes 5-8 sont aussi des sources de la matrice.
- **Matrice à 16 slots** (au lieu de 8), avec beaucoup plus de destinations : enveloppes ADSR amp et filtre, glide, key track, reverb (size, pre-delay, shimmer, width), BBD (tone, wobble), spring, comp, gate, octaver, Color (low/mid/high), arpégiateur (rate, gate, swing, chance), vitesse des 4 LFO.
- **Menus de destination/source en sous-menus** classés par module (OSCILLATORS, FILTER, ENVELOPES, LFO, ARPEGGIATOR, FX RACK > module…).
- **Molette de modulation (CC1)** : routage par défaut molette → LPF (slot 13 de la matrice) dans chaque preset ; modifiable.
- Anciens projets et presets restent compatibles.
