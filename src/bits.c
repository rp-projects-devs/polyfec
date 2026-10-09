/**
 * @file bits.c
 * @brief Manipulation de mots binaires de taille quelconque.
 */

#include "bits.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void check_block_position(int position) {
  if (position < 0 || position >= BIT_BLOCK_SIZE) {
    fprintf(stderr, "Erreur: la position doit etre comprise entre 0 et 63.\n");
    exit(EXIT_FAILURE);
  }
}

static void check_bitword(const BitWord *word) {
  if (word == NULL || word->blocks == NULL || word->bit_len == 0) {
    fprintf(stderr, "Erreur: mot binaire invalide.\n");
    exit(EXIT_FAILURE);
  }
}

static void check_bitword_position(const BitWord *word, int position) {
  check_bitword(word);

  if (position < 0 || (size_t)position >= word->bit_len) {
    fprintf(stderr, "Erreur: position invalide pour ce mot binaire.\n");
    exit(EXIT_FAILURE);
  }
}

uint64_t bit_block_read(int position, uint64_t bits) {
  check_block_position(position);

  return (bits >> position) & UINT64_C(1);
}

uint64_t bit_block_set(int position, uint64_t bits) {
  check_block_position(position);

  return bits | (UINT64_C(1) << position);
}

uint64_t bit_block_mod(int position, uint64_t bits) {
  check_block_position(position);

  return bits ^ (UINT64_C(1) << position);
}

BitWord bitword_from_str(const char *s) {
  BitWord word;
  size_t len;

  if (s == NULL || *s == '\0') {
    fprintf(stderr, "Erreur: la chaine doit etre non vide.\n");
    exit(EXIT_FAILURE);
  }

  len = strlen(s);

  word.bit_len = len;
  word.block_count = (len + BIT_BLOCK_SIZE - 1) / BIT_BLOCK_SIZE;
  word.blocks = calloc(word.block_count, sizeof(uint64_t));

  if (word.blocks == NULL) {
    fprintf(stderr, "Erreur: allocation memoire impossible.\n");
    exit(EXIT_FAILURE);
  }

  for (size_t i = 0; i < len; i++) {
    size_t position;
    size_t block_index;
    size_t offset;

    if (s[i] != '0' && s[i] != '1') {
      fprintf(stderr, "Erreur: la chaine doit contenir seulement 0 ou 1.\n");
      free(word.blocks);
      exit(EXIT_FAILURE);
    }

    position = len - 1 - i;
    block_index = position / BIT_BLOCK_SIZE;
    offset = position % BIT_BLOCK_SIZE;

    if (s[i] == '1') {
      word.blocks[block_index] =
          bit_block_set((int)offset, word.blocks[block_index]);
    }
  }

  return word;
}

void bitword_destroy(BitWord *word) {
  if (word == NULL) {
    return;
  }

  free(word->blocks);
  word->blocks = NULL;
  word->bit_len = 0;
  word->block_count = 0;
}

uint64_t bitword_read(const BitWord *word, int position) {
  size_t block_index;
  size_t offset;

  check_bitword_position(word, position);

  block_index = (size_t)position / BIT_BLOCK_SIZE;
  offset = (size_t)position % BIT_BLOCK_SIZE;

  return bit_block_read((int)offset, word->blocks[block_index]);
}

void bitword_set(BitWord *word, int position) {
  size_t block_index;
  size_t offset;

  check_bitword_position(word, position);

  block_index = (size_t)position / BIT_BLOCK_SIZE;
  offset = (size_t)position % BIT_BLOCK_SIZE;

  word->blocks[block_index] =
      bit_block_set((int)offset, word->blocks[block_index]);
}

void bitword_mod(BitWord *word, int position) {
  size_t block_index;
  size_t offset;

  check_bitword_position(word, position);

  block_index = (size_t)position / BIT_BLOCK_SIZE;
  offset = (size_t)position % BIT_BLOCK_SIZE;

  word->blocks[block_index] =
      bit_block_mod((int)offset, word->blocks[block_index]);
}

char *bitword_to_str(const BitWord *word) {
  char *s;

  check_bitword(word);

  s = malloc(word->bit_len + 1);

  if (s == NULL) {
    fprintf(stderr, "Erreur: allocation memoire impossible.\n");
    exit(EXIT_FAILURE);
  }

  for (size_t i = 0; i < word->bit_len; i++) {
    size_t position;
    size_t block_index;
    size_t offset;
    uint64_t bit;

    position = word->bit_len - 1 - i;
    block_index = position / BIT_BLOCK_SIZE;
    offset = position % BIT_BLOCK_SIZE;

    bit = bit_block_read((int)offset, word->blocks[block_index]);

    s[i] = bit ? '1' : '0';
  }

  s[word->bit_len] = '\0';

  return s;
}

BitWord bitword_create_zero(size_t bit_len) {
  BitWord word;

  if (bit_len == 0) {
    fprintf(stderr, "Erreur: taille de mot binaire invalide.\n");
    exit(EXIT_FAILURE);
  }

  word.bit_len = bit_len;
  word.block_count = (bit_len + BIT_BLOCK_SIZE - 1u) / BIT_BLOCK_SIZE;
  word.blocks = calloc(word.block_count, sizeof(uint64_t));

  if (word.blocks == NULL) {
    fprintf(stderr, "Erreur: allocation memoire impossible.\n");
    exit(EXIT_FAILURE);
  }

  return word;
}

BitWord bitword_clone(const BitWord *word) {
  BitWord copy;

  check_bitword(word);

  copy = bitword_create_zero(word->bit_len);
  memcpy(copy.blocks, word->blocks, word->block_count * sizeof(uint64_t));

  return copy;
}

static uint64_t bit_mask(size_t bit_count) {
  if (bit_count >= BIT_BLOCK_SIZE) {
    return UINT64_MAX;
  }

  return (UINT64_C(1) << bit_count) - UINT64_C(1);
}

void bitword_xor_shifted_value(BitWord *word, uint64_t value,
                               size_t value_bit_len, size_t shift) {
  size_t block_index;
  size_t offset;
  uint64_t masked_value;

  check_bitword(word);

  if (value_bit_len == 0 || value_bit_len > BIT_BLOCK_SIZE) {
    fprintf(stderr, "Erreur: taille de valeur invalide pour le XOR.\n");
    exit(EXIT_FAILURE);
  }

  if (shift > word->bit_len || value_bit_len > word->bit_len - shift) {
    fprintf(stderr, "Erreur: XOR decale hors du mot binaire.\n");
    exit(EXIT_FAILURE);
  }

  masked_value = value & bit_mask(value_bit_len);

  if (masked_value == 0) {
    return;
  }

  block_index = shift / BIT_BLOCK_SIZE;
  offset = shift % BIT_BLOCK_SIZE;

  word->blocks[block_index] ^= masked_value << offset;

  if (offset != 0 && block_index + 1 < word->block_count) {
    word->blocks[block_index + 1] ^= masked_value >> (BIT_BLOCK_SIZE - offset);
  }
}
