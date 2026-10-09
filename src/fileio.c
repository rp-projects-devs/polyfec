/**
 * @file fileio.c
 * @brief Transmission simulée d'un fichier complet.
 */

#include "fileio.h"

#include "bits.h"
#include "codec.h"
#include "error.h"
#include "search.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void apply_errors_on_active_code_bits(BitWord *codeword, double rate) {
  size_t saved_bit_len = codeword->bit_len;

  codeword->bit_len = N;
  error_apply(codeword, rate);
  codeword->bit_len = saved_bit_len;
}

char *file_to_bin_string(const char *filename) {
  FILE *file = fopen(filename, "rb");
  long file_size;
  size_t bit_count;
  char *bin_str;
  size_t position = 0;

  if (file == NULL) {
    raler("fopen");
  }

  CHK(fseek(file, 0, SEEK_END));

  file_size = ftell(file);
  if (file_size < 0) {
    raler("ftell");
  }

  rewind(file);

  bit_count = (size_t)file_size * 8u;
  bin_str = malloc(bit_count + 1u);

  if (bin_str == NULL) {
    raler("malloc");
  }

  for (long i = 0; i < file_size; i++) {
    int byte = fgetc(file);

    if (byte == EOF) {
      free(bin_str);
      raler("fgetc");
    }

    for (int bit = 7; bit >= 0; bit--) {
      bin_str[position] = ((byte >> bit) & 1) ? '1' : '0';
      position++;
    }
  }

  bin_str[position] = '\0';

  if (fclose(file) == EOF) {
    free(bin_str);
    raler("fclose");
  }

  return bin_str;
}

static size_t ceil_div(size_t value, size_t divisor) {
  return (value + divisor - 1u) / divisor;
}

static char *create_padded_block_string(const char *source, size_t source_len,
                                        size_t offset) {
  char *block = malloc(CODEC_DATA_BITS + 1u);

  if (block == NULL) {
    raler("malloc");
  }

  for (size_t i = 0; i < CODEC_DATA_BITS; i++) {
    size_t source_position = offset + i;

    if (source_position < source_len) {
      block[i] = source[source_position];
    } else {
      block[i] = '0';
    }
  }

  block[CODEC_DATA_BITS] = '\0';

  return block;
}

static void copy_decoded_block(char *destination, size_t destination_offset,
                               const char *decoded_block) {
  for (size_t i = 0; i < CODEC_DATA_BITS; i++) {
    destination[destination_offset + i] = decoded_block[i];
  }
}

static void print_bin_string_as_char(const char *bin_str) {
  size_t len = strlen(bin_str);
  unsigned char byte = 0;
  int bit_count = 0;

  for (size_t i = 0; i < len; i++) {
    byte = (unsigned char)(byte << 1);

    if (bin_str[i] == '1') {
      byte = (unsigned char)(byte | 1u);
    }

    bit_count++;

    if (bit_count == 8) {
      putchar(byte);
      byte = 0;
      bit_count = 0;
    }
  }
}

void fileio_process(const char *filename, double rate) {
  char *input_bits = file_to_bin_string(filename);
  size_t input_bit_len = strlen(input_bits);
  size_t block_count = ceil_div(input_bit_len, CODEC_DATA_BITS);
  size_t decoded_capacity = block_count * CODEC_DATA_BITS;
  char *decoded_bits = malloc(decoded_capacity + 1u);
  GeneratorPolynomial generator = search_get_generator_polynomial();

  if (decoded_bits == NULL) {
    free(input_bits);
    raler("malloc");
  }

  if (generator.polynomial == 0) {
    free(input_bits);
    free(decoded_bits);
    fprintf(stderr, "Erreur: aucun polynome generateur valide trouve.\n");
    exit(EXIT_FAILURE);
  }

  for (size_t block_index = 0; block_index < block_count; block_index++) {
    size_t input_offset = block_index * CODEC_DATA_BITS;
    size_t output_offset = block_index * CODEC_DATA_BITS;

    char *block_str =
        create_padded_block_string(input_bits, input_bit_len, input_offset);

    BitWord data_block = bitword_from_str(block_str);
    BitWord codeword = codec_encode_block(&data_block, generator.polynomial);
    BitWord decoded_block;
    char *decoded_block_str;

    free(block_str);

    apply_errors_on_active_code_bits(&codeword, rate);

    codec_correct_block(&codeword, &generator);

    decoded_block = codec_decode_block(&codeword);
    decoded_block_str = bitword_to_str(&decoded_block);

    copy_decoded_block(decoded_bits, output_offset, decoded_block_str);

    free(decoded_block_str);
    bitword_destroy(&decoded_block);
    bitword_destroy(&codeword);
    bitword_destroy(&data_block);
  }

  decoded_bits[input_bit_len] = '\0';

  print_bin_string_as_char(decoded_bits);

  free(decoded_bits);
  free(input_bits);
}
