/**
 * @file codec.c
 * @brief Encodage, correction et décodage d'un bloc.
 */

#include "codec.h"

#include "polynomial.h"

#include <stdio.h>
#include <stdlib.h>

static void codec_check_data_block(const BitWord *data_block) {
  if (data_block == NULL || data_block->blocks == NULL ||
      data_block->bit_len != CODEC_DATA_BITS) {
    fprintf(stderr, "Erreur: bloc de donnees invalide pour le codec.\n");
    exit(EXIT_FAILURE);
  }
}

static void codec_check_codeword(const BitWord *codeword) {
  if (codeword == NULL || codeword->blocks == NULL ||
      codeword->bit_len != CODEC_CODE_BITS) {
    fprintf(stderr, "Erreur: mot de code invalide pour le codec.\n");
    exit(EXIT_FAILURE);
  }
}

BitWord codec_encode_block(const BitWord *data_block,
                           Polynomial generator_poly) {
  BitWord codeword;
  Syndrome remainder;

  codec_check_data_block(data_block);

  codeword = bitword_create_zero(CODEC_CODE_BITS);

  /*
   * Encodage systématique :
   *
   * - les 240 bits utiles sont copiés à partir de la position R ;
   * - les positions 0 à R-1 sont réservées au reste ;
   * - les positions 248 à 254 sont laissées à 0 ;
   * - la position 255 est un bit mort laissé à 0.
   */
  for (size_t i = 0; i < CODEC_DATA_BITS; i++) {
    if (bitword_read(data_block, (int)i)) {
      bitword_set(&codeword, (int)(i + R));
    }
  }

  /*
   * On calcule le reste de la division de data(x) * x^R par G(x).
   * Ce reste est placé dans les R bits de poids faible.
   */
  remainder = polynomial_syndrome(&codeword, generator_poly);

  for (size_t i = 0; i < R; i++) {
    if ((remainder >> i) & 1u) {
      bitword_set(&codeword, (int)i);
    }
  }

  return codeword;
}

BitWord codec_decode_block(const BitWord *codeword) {
  BitWord data_block;

  codec_check_codeword(codeword);

  data_block = bitword_create_zero(CODEC_DATA_BITS);

  /*
   * On récupère uniquement les 240 bits utiles.
   * Les 8 bits de reste, les 7 bits neutralisés et le bit mort sont ignorés.
   */
  for (size_t i = 0; i < CODEC_DATA_BITS; i++) {
    if (bitword_read(codeword, (int)(i + R))) {
      bitword_set(&data_block, (int)i);
    }
  }

  return data_block;
}

int codec_correct_block(BitWord *codeword,
                        const GeneratorPolynomial *generator) {
  Syndrome syndrome;

  codec_check_codeword(codeword);

  if (generator == NULL || generator->polynomial == 0) {
    fprintf(stderr, "Erreur: polynome generateur invalide.\n");
    exit(EXIT_FAILURE);
  }

  syndrome = polynomial_syndrome(codeword, generator->polynomial);

  if (syndrome == 0) {
    return 0;
  }

  for (size_t position = 0; position < N; position++) {
    if (generator->syndromes[position] == syndrome) {
      bitword_mod(codeword, (int)position);
      return 1;
    }
  }

  return -1;
}
