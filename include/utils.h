#ifndef UTILS_H
#define UTILS_H

/**
 * @file utils.h
 * @brief Gestion des erreurs système.
 *
 * Les erreurs d'appels système et de bibliothèque (fopen, malloc, ...) sont
 * fatales dans ce programme : un message est affiché avec perror() puis le
 * programme s'arrête.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @def CHK(op)
 * @brief Vérifie le retour d'une opération qui renvoie -1 en cas d'erreur.
 *
 * Si @p op vaut -1, raler() est appelée avec le texte de l'opération.
 *
 * @param op Opération à vérifier.
 */
#define CHK(op)                                                                \
  do {                                                                         \
    if ((op) == -1)                                                            \
      raler(#op);                                                              \
  } while (0)

/**
 * @brief Affiche un message d'erreur système puis arrête le programme.
 *
 * @param msg Contexte de l'erreur, en général le nom de l'opération.
 */
void raler(const char *msg);

#endif
