/**
 * @file polynomial.c
 * @brief Calculs sur les polynômes binaires dans F2.
 *
 * Dans F2, l'addition et la soustraction correspondent toutes deux au XOR.
 */

#include "polynomial.h"

int poly_degree(Polynomial poly) {
  for (int i = 15; i >= 0; i--) {
    if ((poly >> i) & 1)
      return i;
  }
  return -1;
}

int bitword_degree(const BitWord *word) {
  if (word == NULL || word->bit_len == 0)
    return -1;

  for (int i = (int)word->bit_len - 1; i >= 0; i--) {
    if (bitword_read(word, i) == 1)
      return i;
  }
  return -1;
}

void polynomial_divide(BitWord *word, Polynomial poly) {
  int deg_poly = poly_degree(poly);

  if (word == NULL || deg_poly < 0) {
    return;
  }

  for (size_t pos = word->bit_len; pos > (size_t)deg_poly; pos--) {
    size_t current_position = pos - 1u;

    if (bitword_read(word, (int)current_position) == 0) {
      continue;
    }

    size_t shift = current_position - (size_t)deg_poly;

    bitword_xor_shifted_value(word, (uint64_t)poly, (size_t)deg_poly + 1u,
                              shift);
  }
}

Syndrome polynomial_syndrome(const BitWord *word, Polynomial poly) {
  BitWord copy;
  Syndrome syn;

  if (word == NULL || word->blocks == NULL || word->block_count == 0) {
    return 0;
  }

  copy = bitword_clone(word);

  polynomial_divide(&copy, poly);
  syn = (Syndrome)(copy.blocks[0] & N);

  bitword_destroy(&copy);

  return syn;
}

int polynomial_is_divisible(const BitWord *word, Polynomial poly) {
  return polynomial_syndrome(word, poly) == 0;
}
