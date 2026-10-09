/**
 * @file utils.c
 * @brief Gestion des erreurs système.
 */

#include "utils.h"

void raler(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}
