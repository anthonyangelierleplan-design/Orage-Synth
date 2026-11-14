# Foudre Audio — Base de connaissances plugins

Ce que la construction d'Orage et d'Orage FX a appris, pour que le prochain plugin évite les mêmes pièges.
À relire avant d'écrire un nouveau plugin ; dérouler les tests et la checklist avant chaque envoi.
Les chiffres viennent des tests automatiques, pas d'une écoute : l'oreille reste le dernier juge.

## Architecture type
- JUCE 8.0.4, C++17, build Windows par GitHub Actions, release `latest`.
- `Synth.h` : voix, oscillateurs, filtres, matrice. **Aucune dépendance JUCE** : se teste seul.
- `Engine.h` : rack d'effets, câblage, modulations externes (blocs de contrôle de 32 échantillons).
- `Arp.h` : arpégiateur et pistes de modulation.
- `PluginProcessor.cpp` : paramètres, presets, MIDI, état. Les tables de paramètres sont lues dans l'ordre : l'ordre doit rester identique partout.
- `PluginEditor.cpp`, `SynthEditor.cpp` : interface. Jamais de calcul audio ici.
- Une même source donne deux plugins via `ORAGE_SYNTH` (1 = synthé, 0 = effets seuls).
- Après modification d'un en-tête : recompiler TOUS les objets (un objet périmé = plantage `free(): invalid pointer`).

## Journal des problèmes (symptôme → cause → correction)
- v17 — Aigus parasites wavefolder/drive sur notes hautes → repliement de spectre → suréchantillonnage ×4 (demi-bandes 23 et 11 taps), triangle polyBLAMP.
- v17 — Composante continue en sortie → saturation asymétrique → coupe-continu 6 Hz en sortie du synthé.
- v16 — Escalier sur le cutoff → réglages lus par blocs sans lissage, CC MIDI à 128 pas → 2 lisseurs en cascade (5,5 ms chacun) + coefficients interpolés par échantillon.
- v16 — Aigus audibles filtre fermé avec résonance (MS-20) → bande passante ajoutée à la sortie non filtrée → refiltrée à 2× le cutoff (+42 dB → +6 dB).
- v16 — NaN sur ~1 réglage extrême sur 100 → FM forte sur note très aiguë, phase hors [0,1[ → incréments bornés à 0,49, phase repliée avec floor, garde-fou par voix.
- v16 — Source assignée sans effet la première fois → routage existant à quantité zéro ou LFO très lent → réassigner donne 50 %, point vivant sur l'anneau.
- v15b — Anneaux coupés, chiffres « 82 » collés → dessin hors zone du potard → anneaux entre corps et graduations, chiffres rapprochés.
- v15 — Latence au contrôleur MIDI → réglages du synthé lus avant le MIDI → CC appliqués en début de bloc, une fois par bloc.
- v15 — Drive qui assombrit → saturation large bande → accentuation > 1,2 kHz avant saturation.
- v14c — Molette non assignable en MIDI learn → CC1 exclu → accepté.
- v14 — Molette sans effet → aucun routage par défaut → molette → LPF (slot 13) + moniteur MIDI.
- v13 — Macros sur de mauvais CC → numéros sauvegardés dans les presets → déplacés dans le projet, défaut CC 16–23.
- v13 — Tuiles du rack mal alignées → deux listes de modules → une liste partagée.

## Règles de qualité audio
1. Lisser tout réglage continu (2 étages, ~20 ms au total).
2. Interpoler les coefficients à chaque échantillon.
3. Suréchantillonner toute saturation forte (×4).
4. Band-limiter les oscillateurs (polyBLEP, polyBLAMP).
5. Borner ce qui peut exploser (phase, résonance, sortie en tanh).
6. Garde-fou par voix : valeur non finie → états remis à zéro.
7. Un filtre fermé doit être vraiment fermé : tout signal ajouté après le filtre est lui-même filtré.
8. Coupe-continu en sortie dès qu'une saturation est asymétrique.
9. Lire le MIDI avant les réglages du bloc.
10. Démarrer neutre (volume prudent, effets à zéro).

## MIDI, presets, compatibilité
- Ne jamais réordonner une liste enregistrée (sources, destinations, modules) : ajouter à la fin.
- Paramètres identifiés par leur nom ; un paramètre absent prend sa valeur par défaut.
- Garder le chargement des anciens formats.
- Assignations MIDI dans le projet et la configuration globale, pas dans les presets.
- Macros CC 16–23, molette CC1, apprentissage MIDI par clic droit, modes SCALE / PICKUP / DIRECT.
- Un moniteur MIDI visible dans l'en-tête.

## Interface
- Rien ne dépasse de la zone d'un potard ; vérifier les voisins sur capture agrandie.
- Menus longs en sous-menus par module.
- Modulation : clic droit « Assigner à », 2 sources max, un anneau par source, Ctrl + clic gauche + glisser.
- Montrer une capture avant d'envoyer ; attendre l'accord avant un nouveau design.
- Non testable sur le banc : menus ouverts, souris réelle.

## Batterie de tests (résultats Orage v17)
- Réglages aléatoires extrêmes : 0 NaN sur 250, pic max 0,84.
- 65 presets d'usine : pic < 1, continu < 0,02 ; 5 signalés pour souffle ou larsen voulu.
- Filtre fermé + résonance : +6 dB d'aigus au plus.
- Repliement : drive −38 dB (−22 avant), wavefolder à fond −20 dB (−11 avant).
- CPU : 8 % d'un cœur, 6 notes, rack complet, compilation optimisée.
- Apprentissage MIDI, état sauvegardé, captures agrandies.
- Méthode : un test qui ne distingue pas avant/après mesure la mauvaise chose ; pour un NaN, remettre un réglage à zéro à la fois.

## Checklist avant de pousser
1. Compilation synthé sans erreur, syntaxe de la version effets.
2. Tous les objets recompilés.
3. Batterie de tests.
4. Listes enregistrées intactes.
5. Capture agrandie regardée.
6. README à jour.
7. Accord donné.
8. Push sur les deux dépôts, deux builds verts.
9. Compte rendu : mesuré, non vérifié, à juger à l'oreille.

## Reste à faire sur Orage
- À l'oreille : lissage 20 ms, MS-20 adouci, drive brillant, wavefolder.
- Dans Ableton : menu « Assigner à », anneaux à la souris, sous-menus.
- Wavefolder à fond sur notes très aiguës : reste −20 dB (×8 possible).
- Rack FX : saturations non suréchantillonnées, pas d'audit à l'écoute module par module.
- Clics possibles sur changements à positions fixes pendant une note.
