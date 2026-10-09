/**
 * @file error.c
 * @brief Simulation d'un canal bruité.
 */

#include "error.h"

#include <stdlib.h>

void error_apply(BitWord *word, double rate) {
  if (word == NULL || rate <= 0.0) {
    return;
  }

  if (rate > 1.0) {
    rate = 1.0;
  }

  for (size_t i = 0; i < word->bit_len; i++) {
    double rand_val = (double)rand() / (double)RAND_MAX;

    if (rand_val < rate) {
      bitword_mod(word, (int)i);
    }
  }
}

void error_apply_only_one(BitWord *word) {
  size_t position;

  if (word == NULL || word->bit_len == 0) {
    return;
  }

  position = (size_t)rand() % word->bit_len;
  bitword_mod(word, (int)position);
}
