#ifndef ERROR_H
#define ERROR_H

#include "bits.h"

/**
 * @file error.h
 * @brief Simulation d'un canal bruité.
 *
 * Les erreurs sont tirées avec rand(). Le générateur est initialisé une fois
 * au démarrage du programme (voir main.c).
 */

/**
 * @brief Inverse chaque bit d'un mot avec une probabilité donnée.
 *
 * Pour chaque bit, un nombre aléatoire entre 0 et 1 est tiré. Si ce nombre
 * est inférieur au taux, le bit est inversé.
 *
 * @param word Mot binaire à modifier.
 * @param rate Taux d'erreur binaire, entre 0.0 et 1.0. Un taux nul ne
 *             modifie pas le mot.
 */
void error_apply(BitWord *word, double rate);

/**
 * @brief Inverse exactement un bit, choisi au hasard dans tout le mot.
 *
 * @param word Mot binaire à modifier.
 */
void error_apply_only_one(BitWord *word);

#endif
