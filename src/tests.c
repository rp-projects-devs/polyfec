/**
 * @file tests.c
 * @brief Tests unitaires intégrés au programme.
 */

#include "tests.h"

#include "bits.h"
#include "codec.h"
#include "error.h"
#include "polynomial.h"
#include "search.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tests_failed = 0;

static void test_assert(int condition, const char *message) {
  if (!condition) {
    fprintf(stderr, "[FAIL] %s\n", message);
    tests_failed++;
  } else {
    printf("[OK] %s\n", message);
  }
}

static char *make_data_pattern(void) {
  char *s = malloc(CODEC_DATA_BITS + 1u);

  if (s == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  for (size_t i = 0; i < CODEC_DATA_BITS; i++) {
    s[i] = (i % 2u == 0u) ? '1' : '0';
  }

  s[CODEC_DATA_BITS] = '\0';
  return s;
}

static void test_bits(void) {
  BitWord word;
  char *result;

  word = bitword_from_str("101010");

  test_assert(bitword_read(&word, 0) == 0, "read position 0");
  test_assert(bitword_read(&word, 1) == 1, "read position 1");

  bitword_set(&word, 0);
  result = bitword_to_str(&word);
  test_assert(strcmp(result, "101011") == 0, "set position 0");
  free(result);

  bitword_mod(&word, 1);
  result = bitword_to_str(&word);
  test_assert(strcmp(result, "101001") == 0, "mod position 1");
  free(result);

  bitword_destroy(&word);
}

static void test_bits_multi_block(void) {
  /* Mot de 130 bits : trois blocs de 64 bits. */
  BitWord word = bitword_create_zero(130);
  char *result;

  bitword_set(&word, 63);
  bitword_set(&word, 64);
  bitword_set(&word, 129);

  test_assert(word.block_count == 3, "mot de 130 bits : 3 blocs");
  test_assert(bitword_read(&word, 63) == 1 && bitword_read(&word, 64) == 1 &&
                  bitword_read(&word, 129) == 1 && bitword_read(&word, 65) == 0,
              "lecture de part et d'autre d'une frontiere de bloc");

  result = bitword_to_str(&word);
  test_assert(result[0] == '1' && result[129 - 64] == '1' &&
                  result[129 - 63] == '1',
              "conversion en chaine d'un mot multi-blocs");
  free(result);

  /* XOR a cheval sur deux blocs : 0b111 decale de 62 touche 62, 63 et 64. */
  bitword_xor_shifted_value(&word, 0x7, 3, 62);
  test_assert(bitword_read(&word, 62) == 1 && bitword_read(&word, 63) == 0 &&
                  bitword_read(&word, 64) == 0,
              "XOR decale a cheval sur deux blocs");

  bitword_destroy(&word);
}

static void test_degrees(void) {
  BitWord word = bitword_from_str("0001011");
  BitWord zero = bitword_create_zero(10);

  test_assert(poly_degree(0x11D) == 8, "degre de 0x11D = 8");
  test_assert(poly_degree(0) == -1, "degre du polynome nul = -1");
  test_assert(bitword_degree(&word) == 3, "degre du mot 0001011 = 3");
  test_assert(bitword_degree(&zero) == -1, "degre d'un mot nul = -1");

  bitword_destroy(&zero);
  bitword_destroy(&word);
}

static void test_polynomial_division(void) {
  /*
   * Exemple :
   * 100101 = x^5 + x^2 + 1
   * 1011   = x^3 + x + 1
   *
   * Le reste attendu est 10 = x.
   */
  BitWord word = bitword_from_str("100101");
  Polynomial poly = 0x0B;
  char *result;

  polynomial_divide(&word, poly);
  result = bitword_to_str(&word);

  test_assert(strcmp(result, "000010") == 0, "division polynomiale : reste x");

  free(result);
  bitword_destroy(&word);
}

static void test_search(void) {
  GeneratorPolynomial generator = search_get_generator_polynomial();
  size_t valid_count = search_valid_generator_polynomials(NULL, 0);

  test_assert(generator.polynomial != 0,
              "search : un polynome generateur est trouve");

  test_assert(generator.polynomial == 0x11D,
              "search : premier polynome attendu 0x11D");

  /* Les generateurs valides sont les 16 polynomes primitifs de degre 8. */
  test_assert(valid_count == 16, "search : 16 polynomes generateurs valides");
}

static void test_codec_roundtrip(void) {
  GeneratorPolynomial generator = search_get_generator_polynomial();
  char *data_str = make_data_pattern();
  BitWord data = bitword_from_str(data_str);
  BitWord codeword = codec_encode_block(&data, generator.polynomial);
  BitWord decoded = codec_decode_block(&codeword);
  char *decoded_str = bitword_to_str(&decoded);

  test_assert(polynomial_is_divisible(&codeword, generator.polynomial),
              "codec : le mot encode est divisible par G(x)");

  test_assert(strcmp(data_str, decoded_str) == 0,
              "codec : decode(encode(data)) = data");

  free(decoded_str);
  bitword_destroy(&decoded);
  bitword_destroy(&codeword);
  bitword_destroy(&data);
  free(data_str);
}

static void test_codec_single_error_correction(void) {
  GeneratorPolynomial generator = search_get_generator_polynomial();
  char *data_str = make_data_pattern();
  int all_positions_ok = 1;

  for (size_t error_position = 0; error_position < N; error_position++) {
    BitWord data = bitword_from_str(data_str);
    BitWord codeword = codec_encode_block(&data, generator.polynomial);
    BitWord decoded;
    char *decoded_str;
    int correction_status;

    bitword_mod(&codeword, (int)error_position);

    correction_status = codec_correct_block(&codeword, &generator);
    decoded = codec_decode_block(&codeword);
    decoded_str = bitword_to_str(&decoded);

    if (correction_status != 1 || strcmp(data_str, decoded_str) != 0) {
      all_positions_ok = 0;
    }

    free(decoded_str);
    bitword_destroy(&decoded);
    bitword_destroy(&codeword);
    bitword_destroy(&data);
  }

  test_assert(
      all_positions_ok,
      "codec : correction d'une erreur simple sur chaque position active");

  free(data_str);
}

static void test_error_channel(void) {
  GeneratorPolynomial generator = search_get_generator_polynomial();
  char *data_str = make_data_pattern();
  BitWord data = bitword_from_str(data_str);
  BitWord codeword = codec_encode_block(&data, generator.polynomial);
  BitWord received = bitword_clone(&codeword);
  BitWord decoded;
  char *decoded_str;
  size_t differences = 0;

  error_apply(&received, 0.0);
  test_assert(memcmp(received.blocks, codeword.blocks,
                     codeword.block_count * sizeof(uint64_t)) == 0,
              "canal : un taux nul ne modifie pas le mot");

  /* Une erreur aleatoire sur les 255 positions actives. */
  received.bit_len = N;
  error_apply_only_one(&received);
  received.bit_len = CODEC_CODE_BITS;

  for (int i = 0; i < (int)CODEC_CODE_BITS; i++) {
    differences += bitword_read(&received, i) != bitword_read(&codeword, i);
  }
  test_assert(differences == 1, "canal : exactement un bit inverse");

  test_assert(codec_correct_block(&received, &generator) == 1,
              "codec : erreur aleatoire detectee et corrigee");

  decoded = codec_decode_block(&received);
  decoded_str = bitword_to_str(&decoded);
  test_assert(strcmp(decoded_str, data_str) == 0,
              "codec : donnees retrouvees apres une erreur aleatoire");

  free(decoded_str);
  bitword_destroy(&decoded);
  bitword_destroy(&received);
  bitword_destroy(&codeword);
  bitword_destroy(&data);
  free(data_str);
}

int tests_run_all(void) {
  tests_failed = 0;

  printf("Lancement des tests internes\n");

  test_bits();
  test_bits_multi_block();
  test_degrees();
  test_polynomial_division();
  test_search();
  test_codec_roundtrip();
  test_codec_single_error_correction();
  test_error_channel();

  if (tests_failed != 0) {
    fprintf(stderr, "\n%d test(s) echoue(s).\n", tests_failed);
    return EXIT_FAILURE;
  }

  printf("\nTous les tests sont passes.\n");
  return EXIT_SUCCESS;
}
