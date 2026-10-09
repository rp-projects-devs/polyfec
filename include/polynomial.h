#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include "bits.h"
#include "search.h"

/**
 * @file polynomial.h
 * @brief Fonctions de calcul sur les polynômes binaires dans F2.
 *
 * Ce module fournit les opérations nécessaires au calcul du syndrome :
 * degré d'un polynôme, division polynomiale modulo 2, reste de division
 * et test de divisibilité.
 *
 * Dans F2, les coefficients valent 0 ou 1. L'addition et la soustraction
 * sont donc équivalentes à un XOR.
 */

/**
 * @brief Calcule le degré d'un polynôme binaire.
 *
 * Le polynôme est stocké dans un entier. Le bit i indique si le terme x^i
 * est présent. Le degré correspond donc à la position du bit de poids fort
 * valant 1.
 *
 * @param poly Polynôme à analyser.
 *
 * @return Le degré du polynôme, ou -1 si le polynôme est nul.
 */
int poly_degree(Polynomial poly);

/**
 * @brief Calcule le degré d'un mot binaire vu comme polynôme.
 *
 * Le mot est interprété comme un polynôme binaire : le bit en position i
 * représente le coefficient du terme x^i. Le degré correspond donc à la
 * plus grande position contenant un bit à 1.
 *
 * @param word Mot binaire à analyser.
 *
 * @return Le degré du mot, ou -1 si le mot est nul ou invalide.
 */
int bitword_degree(const BitWord *word);

/**
 * @brief Divise un mot binaire par un polynôme dans F2.
 *
 * La fonction applique une division euclidienne polynomiale modulo 2.
 * À chaque étape, le polynôme diviseur est aligné sur le terme dominant
 * du mot courant, puis soustrait. Dans F2, cette soustraction est réalisée
 * par des XOR, donc par inversion des bits concernés.
 *
 * La division est destructive : à la fin de la fonction, word contient
 * uniquement le reste de la division.
 *
 * @param word Mot binaire à diviser. Il est modifié en place.
 * @param poly Polynôme diviseur.
 */
void polynomial_divide(BitWord *word, Polynomial poly);

/**
 * @brief Calcule le syndrome d'un mot binaire.
 *
 * Le syndrome est le reste de la division polynomiale du mot par le
 * polynôme donné. La fonction travaille sur une copie du mot afin de ne
 * pas modifier le mot original.
 *
 * Ici, le syndrome tient sur R bits, donc sur un octet lorsque
 * R vaut 8.
 *
 * @param word Mot binaire dont on veut calculer le syndrome.
 * @param poly Polynôme générateur utilisé pour la division.
 *
 * @return Le syndrome du mot.
 */
Syndrome polynomial_syndrome(const BitWord *word, Polynomial poly);

/**
 * @brief Teste si un mot binaire est divisible par un polynôme.
 *
 * Un mot est divisible par le polynôme générateur si le reste de sa division
 * est nul. Dans le contexte du code correcteur, cela signifie que le mot est
 * un mot de code valide.
 *
 * @param word Mot binaire à tester.
 * @param poly Polynôme générateur.
 *
 * @return 1 si le mot est divisible par poly, 0 sinon.
 */
int polynomial_is_divisible(const BitWord *word, Polynomial poly);

#endif
