# Théorie et choix de conception

Ce document explique les mathématiques du code et les choix d'implémentation. Le [README](../README.md) décrit l'utilisation du programme.

## Dimensions du code

Le syndrome tient sur un octet : **R = 8** bits de redondance, donc 2⁸ = 256 syndromes possibles. Le syndrome nul signifie « aucune erreur ». Il reste 255 syndromes non nuls, ce qui permet d'identifier au plus 255 positions d'erreur simple.

Le code théorique optimal utilise donc tous les syndromes disponibles :

$$
N = 2^8 - 1 = 255, \qquad K = N - R = 247, \qquad \eta = \frac{247}{255} \approx 96{,}86\ \%
$$

Chaque position d'erreur simple possède un syndrome non nul distinct. Le code a donc une distance minimale d'au moins 3, ce qui est la condition pour corriger une erreur.

L'implémentation range chaque mot de code dans un conteneur de **256 bits** et transporte **240 bits de données**, soit exactement 30 octets :

$$
N_{\text{impl}} = 256, \qquad K_{\text{impl}} = 240, \qquad \eta_{\text{impl}} = \frac{240}{256} = 93{,}75\ \%
$$

On perd environ 3 points de rendement, mais un fichier se découpe alors en blocs d'octets entiers, sans manipulation de bits à cheval sur deux blocs.

| Positions | Contenu |
|---|---|
| 0 à 7 | Redondance : reste de la division par G(x) |
| 8 à 247 | 240 bits de données |
| 248 à 254 | Bits neutralisés, toujours à 0 |
| 255 | Bit mort, hors du code |

Les positions 0 à 254 forment le code théorique de longueur 255 : une erreur sur n'importe laquelle d'entre elles est corrigée. La position 255 n'a pas de syndrome associé ; le canal simulé ne la modifie donc jamais.

## Représentation des données

### Mots binaires

Un mot de longueur quelconque est stocké dans un tableau de blocs de 64 bits (`BitWord`). La **position 0 est le bit de poids faible**, le plus à droite quand le mot est écrit. Cette convention est naturelle pour les polynômes : le bit de position *i* est le coefficient de xⁱ.

Une position se décompose en un numéro de bloc et un décalage dans ce bloc :

$$
\text{bloc} = \left\lfloor \frac{\text{position}}{64} \right\rfloor, \qquad \text{décalage} = \text{position} \bmod 64
$$

Les trois opérations élémentaires sont des masques :

| Opération | Calcul |
|---|---|
| Lire | `(bloc >> i) & 1` |
| Mettre à 1 | `bloc \| (1 << i)` |
| Inverser | `bloc ^ (1 << i)` |

L'inversion par XOR correspond exactement à l'addition dans F₂, le corps à deux éléments sur lequel travaillent les polynômes du code.

### Polynômes

Un polynôme binaire de petit degré tient dans un entier de 16 bits (`Polynomial`) : le bit *i* indique la présence du terme xⁱ.

$$
\texttt{0x11D} = 1\,0001\,1101_2 = x^8 + x^4 + x^3 + x^2 + 1
$$

Multiplier par x revient à décaler d'un bit vers la gauche, et soustraire un polynôme revient à faire un XOR.

## Recherche du polynôme générateur

Soit C(x) un mot de code valide, E(x) l'erreur introduite par le canal et M(x) = C(x) + E(x) le mot reçu. Comme C(x) est divisible par le polynôme générateur G(x) :

$$
M(x) \bmod G(x) = E(x) \bmod G(x)
$$

**Le syndrome ne dépend que de l'erreur**, pas du message. Pour une erreur simple en position *i*, E(x) = xⁱ. Pour pouvoir corriger toute erreur simple, il faut donc que les 255 valeurs

$$
x^0 \bmod G(x),\ x^1 \bmod G(x),\ \ldots,\ x^{254} \bmod G(x)
$$

soient **non nulles et deux à deux distinctes**. Si deux positions avaient le même syndrome, on ne saurait pas quel bit corriger ; si une erreur avait un syndrome nul, elle passerait inaperçue.

Les candidats sont les polynômes de degré 8 (bit 8 à 1) avec un terme constant (bit 0 à 1), c'est-à-dire les polynômes impairs de `0x101` à `0x1FF` : 2⁷ = 128 candidats.

Pour remplir la table des syndromes d'un candidat, on n'effectue pas 255 divisions complètes. On utilise la récurrence :

$$
S_0 = 1, \qquad S_{i+1} = x \cdot S_i \bmod G(x)
$$

Multiplier par x est un décalage à gauche ; si le résultat atteint le degré 8, on le réduit par un XOR avec G(x). Un tableau de marquage détecte un syndrome nul ou déjà vu, ce qui élimine le candidat. La recherche complète coûte 128 × 255 itérations, soit quelques millisecondes.

La recherche trouve **16 polynômes valides**. Ce sont exactement les polynômes primitifs de degré 8 : un polynôme convient si et seulement si x est d'ordre 255 modulo G(x), ce qui est la définition d'un polynôme primitif. Le code obtenu est un **code de Hamming cyclique (255, 247)**.

Le programme retient le premier, `0x11D` = x⁸ + x⁴ + x³ + x² + 1. C'est aussi le polynôme utilisé par les codes Reed-Solomon des QR codes.

## Encodage, correction, décodage

### Division polynomiale

C'est une division euclidienne dans F₂. On parcourt le mot depuis le bit de poids fort ; chaque fois qu'un bit vaut 1 au-dessus du degré de G(x), on aligne G(x) sur ce bit et on applique un XOR, ce qui l'annule. À la fin, il ne reste que les 8 bits de poids faible : le reste de la division.

### Encodage systématique

1. Les 240 bits de données sont placés à partir de la position 8, ce qui revient à calculer data(x) · x⁸.
2. On calcule le reste de la division de ce mot par G(x).
3. Ce reste est écrit dans les positions 0 à 7.

Le mot obtenu est divisible par G(x) : son syndrome est nul. Les données restent lisibles telles quelles dans le mot de code, d'où le terme « systématique ».

### Correction

On calcule le syndrome S = M(x) mod G(x) du mot reçu :

- **S = 0** : aucune erreur détectée ;
- **S est dans la table** : la position associée désigne le bit fautif, que l'on inverse ;
- **S n'est pas dans la table** : l'erreur n'est pas une erreur simple. Avec ce code de longueur maximale, ce cas ne se produit pas en pratique, car les 255 syndromes non nuls correspondent tous à une position.

### Décodage

On extrait les positions 8 à 247. Les bits de redondance, les bits neutralisés et le bit mort sont ignorés.

## Transmission d'un fichier

1. Le fichier est lu octet par octet ; chaque octet devient 8 bits, poids fort en premier.
2. Le flux de bits est découpé en blocs de 240 bits ; le dernier est complété par des zéros.
3. Chaque bloc est encodé, traverse le canal simulé, puis est corrigé et décodé.
4. Les bits de remplissage sont retirés grâce à la taille d'origine, et les octets sont écrits sur la sortie standard.

Le canal inverse chaque bit des positions 0 à 254 avec une probabilité égale au taux donné. Le générateur aléatoire est initialisé avec l'heure au démarrage.

## Limites

Le code **corrige une seule erreur par bloc**. Avec deux erreurs ou plus, le syndrome correspond presque toujours à une autre position : le décodeur « corrige » alors un bit sain et ajoute une erreur au lieu d'en retirer.

La probabilité qu'un bloc reste faux se calcule directement. Pour un taux d'erreur binaire *p*, un bloc non protégé de 240 bits est faux dès qu'un bit est touché ; un bloc protégé de 255 bits actifs n'est faux qu'à partir de deux erreurs :

$$
P_{\text{sans code}} = 1 - (1-p)^{240}, \qquad P_{\text{avec code}} = 1 - (1-p)^{255} - 255\,p\,(1-p)^{254}
$$

| Taux d'erreur binaire *p* | Blocs faux sans code | Blocs faux avec code |
|---|---|---|
| 0,0001 | 2,37 % | 0,03 % |
| 0,001 | 21,35 % | 2,74 % |
| 0,003 | 51,38 % | 17,86 % |
| 0,01 | 91,04 % | 72,44 % |

Le code est très efficace sur un canal peu bruité, où il divise le taux de blocs faux par un facteur de 8 à 80. Il perd son intérêt dès que plusieurs erreurs par bloc deviennent fréquentes ; il faudrait alors un code plus puissant (BCH, Reed-Solomon) ou des blocs plus courts.
