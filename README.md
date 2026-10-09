# polyfec

[![CI](https://github.com/rp-projects-devs/polyfec/actions/workflows/ci.yml/badge.svg)](https://github.com/rp-projects-devs/polyfec/actions/workflows/ci.yml)

**Code correcteur d'erreurs polynomial en C**, avec un syndrome sur un octet : le programme cherche lui-même son polynôme générateur, encode des données, simule un canal bruité et corrige les erreurs à la réception.

Le code obtenu est un code de Hamming cyclique (255, 247) : chaque bloc de 30 octets reçoit 1 octet de redondance, et toute erreur d'un bit par bloc est corrigée.

```console
$ ./polyfec tests/data/message.txt 0.003
Un code correcteur ajoute de la redondance a un message pour que le
destinataire puisse detecter et reparer les erreurs introduites par le canal
de pransmission, sans avoir a redemander les donnees.
```

Ici, environ 0,3 % des bits ont été inversés pendant la transmission simulée (les erreurs sont tirées au hasard, la sortie change à chaque exécution). Presque tous ont été corrigés ; le « p » de « pransmission » vient d'un bloc qui a reçu deux erreurs, au-delà de ce que le code sait réparer.

## Principe

1. **Recherche du générateur.** Parmi les 128 polynômes candidats de degré 8, le programme garde ceux pour lesquels les 255 erreurs simples possibles donnent 255 syndromes distincts et non nuls. Il en trouve 16 (les polynômes primitifs de degré 8) et retient `0x11D` = x⁸ + x⁴ + x³ + x² + 1.
2. **Encodage.** Chaque bloc de 240 bits est décalé de 8 positions, puis complété par le reste de sa division par G(x). Le mot de code de 256 bits obtenu est divisible par G(x).
3. **Canal bruité.** Chaque bit est inversé avec une probabilité donnée.
4. **Correction.** Le reste de la division du mot reçu, le *syndrome*, ne dépend que de l'erreur. S'il est non nul, une table indique directement la position du bit fautif, que l'on inverse.
5. **Décodage.** On extrait les 240 bits de données.

La démarche complète, avec les démonstrations et le calcul des performances, est dans [`docs/theorie.md`](docs/theorie.md).

| Taux d'erreur binaire | Blocs faux sans code | Blocs faux avec code |
|---|---|---|
| 0,0001 | 2,37 % | 0,03 % |
| 0,001 | 21,35 % | 2,74 % |
| 0,01 | 91,04 % | 72,44 % |

## Compilation

Prérequis : un compilateur C17 (gcc ou clang) et `make`, sous Linux ou macOS.

```sh
make            # produit ./polyfec
make test       # tests unitaires + tests de la ligne de commande
```

Autres cibles : `make sanitize` (AddressSanitizer et UBSan), `make valgrind` (fuites mémoire), `make format` et `make check-format` (clang-format), `make help`.

## Utilisation

Les mots binaires s'écrivent avec des `0` et des `1`, la **position 0 étant le bit le plus à droite**. Les polynômes s'écrivent en hexadécimal (`0x11D`) ou en binaire (`100011101`).

| Commande | Rôle |
|---|---|
| `polyfec <fichier> <taux>` | Transmet un fichier sur un canal bruité et affiche le résultat corrigé |
| `polyfec code <poly> <motinfo>` | Encode un bloc ; les données sont complétées à 240 bits par des zéros à droite |
| `polyfec decode <poly> <motcode>` | Extrait les 240 bits de données d'un mot de code de 256 bits |
| `polyfec corrige <poly> <motcode>` | Corrige une erreur simple dans un mot de code de 256 bits |
| `polyfec error <taux> <mot>` | Inverse chaque bit avec la probabilité donnée |
| `polyfec read <pos> <mot>` | Lit un bit |
| `polyfec set <pos> <mot>` | Met un bit à 1 |
| `polyfec mod <pos> <mot>` | Inverse un bit |
| `polyfec search4poly` | Affiche les polynômes générateurs valides et la table des syndromes |
| `polyfec test` | Lance les tests unitaires |

Le taux est compris entre `0` et `1`. Avec un taux nul, la sortie est identique au fichier d'entrée, y compris pour un fichier binaire.

### Exemple : encoder, abîmer, corriger

```sh
mot=$(./polyfec code 0x11D 10100010)       # mot de code de 256 bits
abime=$(./polyfec mod 100 "$mot")          # inverse le bit 100
./polyfec corrige 0x11D "$abime"           # retrouve exactement "$mot"
./polyfec decode 0x11D "$mot"              # 10100010 suivi de 232 zéros
```

## Organisation du code

```
include/ et src/
├── bits        mots binaires de taille quelconque (blocs de 64 bits)
├── polynomial  degré, division dans F2, syndrome
├── search      recherche des polynômes générateurs et table des syndromes
├── codec       encodage systématique, correction, décodage d'un bloc
├── error       canal bruité
├── fileio      transmission d'un fichier complet, bloc par bloc
├── tests       tests unitaires (polyfec test)
├── utils       gestion des erreurs système
└── main        analyse de la ligne de commande
tests/
├── cli_tests.sh   tests de bout en bout de la ligne de commande
└── data/          fichier d'exemple
docs/theorie.md    théorie et choix de conception
```

Chaque fonction publique est documentée dans son en-tête au format Doxygen. Le code compile sans avertissement avec `-Wall -Wextra -Werror -pedantic`.

## Qualité

- **23 tests unitaires** : opérations sur les bits, y compris à la frontière entre deux blocs de 64 bits ; division polynomiale ; recherche des générateurs ; aller-retour encodage-décodage ; correction d'une erreur sur chacune des 255 positions ; canal bruité.
- **29 tests de la ligne de commande** : encodage et correction, transmission de fichiers texte, binaire et vide, rejet des entrées invalides.
- **Intégration continue** GitHub Actions : compilation avec gcc et clang, tests, AddressSanitizer et UBSan, valgrind, vérification du formatage.

## Limites

Le code corrige **une seule erreur par bloc de 256 bits**. Avec deux erreurs dans un même bloc, le syndrome désigne une troisième position, et la « correction » ajoute une erreur au lieu d'en retirer. Le code est donc adapté à un canal peu bruité ; au-delà d'environ 0,1 % de bits erronés, un code plus puissant (BCH, Reed-Solomon) serait nécessaire.

## Licence

MIT, voir [`LICENSE`](LICENSE).
