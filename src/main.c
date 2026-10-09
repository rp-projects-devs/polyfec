/**
 * @file main.c
 * @brief Point d'entrée : analyse de la ligne de commande.
 */

#include "bits.h"
#include "codec.h"
#include "error.h"
#include "fileio.h"
#include "polynomial.h"
#include "search.h"
#include "tests.h"

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void print_usage(const char *prog_name) {
  static const char *const commands[] = {
      "<fichier> <taux>          transmet un fichier sur un canal bruite",
      "error <taux> <mot>        ajoute des erreurs aleatoires",
      "code <poly> <motinfo>     encode un bloc (240 bits max)",
      "decode <poly> <motcode>   extrait les donnees d'un mot de 256 bits",
      "corrige <poly> <motcode>  corrige une erreur simple",
      "read <pos> <mot>          lit un bit",
      "set <pos> <mot>           met un bit a 1",
      "mod <pos> <mot>           inverse un bit",
      "search4poly               cherche les polynomes generateurs",
      "test                      lance les tests unitaires",
  };

  fprintf(stderr, "polyfec : code polynomial correcteur d'erreurs "
                  "(syndrome sur 8 bits)\n\nUsage:\n");

  for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
    fprintf(stderr, "  %s %s\n", prog_name, commands[i]);
  }
}

static double parse_rate(const char *rate_str) {
  char *end;
  double rate;

  if (rate_str == NULL || *rate_str == '\0') {
    fprintf(stderr, "Erreur: taux invalide.\n");
    exit(EXIT_FAILURE);
  }

  errno = 0;
  rate = strtod(rate_str, &end);

  if (errno != 0 || *end != '\0' || rate < 0.0 || rate > 1.0) {
    fprintf(stderr, "Erreur: taux invalide (doit etre entre 0.0 et 1.0).\n");
    exit(EXIT_FAILURE);
  }

  return rate;
}

static int parse_position(const char *s) {
  char *end;
  long value;

  if (s == NULL || *s == '\0') {
    fprintf(stderr, "Erreur: position invalide.\n");
    exit(EXIT_FAILURE);
  }

  errno = 0;
  value = strtol(s, &end, 10);

  if (errno != 0 || *end != '\0' || value < 0 || value > INT_MAX) {
    fprintf(stderr, "Erreur: position invalide.\n");
    exit(EXIT_FAILURE);
  }

  return (int)value;
}

static int is_binary_string(const char *s) {
  if (s == NULL || *s == '\0') {
    return 0;
  }

  for (size_t i = 0; s[i] != '\0'; i++) {
    if (s[i] != '0' && s[i] != '1') {
      return 0;
    }
  }

  return 1;
}

static Polynomial parse_polynomial(const char *poly_str) {
  unsigned long value = 0;

  if (poly_str == NULL || *poly_str == '\0') {
    fprintf(stderr, "Erreur: polynome invalide.\n");
    exit(EXIT_FAILURE);
  }

  if (strlen(poly_str) > 2 && poly_str[0] == '0' &&
      (poly_str[1] == 'x' || poly_str[1] == 'X')) {
    char *end;

    errno = 0;
    value = strtoul(poly_str, &end, 16);

    if (errno != 0 || *end != '\0' || value > UINT16_MAX) {
      fprintf(stderr, "Erreur: polynome invalide.\n");
      exit(EXIT_FAILURE);
    }

    return (Polynomial)value;
  }

  if (!is_binary_string(poly_str)) {
    fprintf(
        stderr,
        "Erreur: polynome invalide, utiliser une chaine binaire ou 0x...\n");
    exit(EXIT_FAILURE);
  }

  for (size_t i = 0; poly_str[i] != '\0'; i++) {
    value = (value << 1u) | (unsigned long)(poly_str[i] - '0');

    if (value > UINT16_MAX) {
      fprintf(stderr, "Erreur: polynome trop grand.\n");
      exit(EXIT_FAILURE);
    }
  }

  return (Polynomial)value;
}

static char *pad_binary_string_right(const char *input, size_t target_len) {
  size_t input_len;
  char *result;

  if (!is_binary_string(input)) {
    fprintf(stderr, "Erreur: mot binaire invalide.\n");
    exit(EXIT_FAILURE);
  }

  input_len = strlen(input);

  if (input_len > target_len) {
    fprintf(stderr, "Erreur: mot trop long.\n");
    exit(EXIT_FAILURE);
  }

  result = malloc(target_len + 1u);

  if (result == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  memcpy(result, input, input_len);
  memset(result + input_len, '0', target_len - input_len);
  result[target_len] = '\0';

  return result;
}

static GeneratorPolynomial get_generator_from_poly(Polynomial polynomial) {
  GeneratorPolynomial generators[GENERATOR_CANDIDATE_COUNT];
  size_t count =
      search_valid_generator_polynomials(generators, GENERATOR_CANDIDATE_COUNT);

  for (size_t i = 0; i < count; i++) {
    if (generators[i].polynomial == polynomial) {
      return generators[i];
    }
  }

  fprintf(stderr, "Erreur: le polynome donne n'est pas un generateur valide "
                  "trouve par search.\n");
  exit(EXIT_FAILURE);
}

static int handle_bit_read(const char *pos_str, const char *word_str) {
  int position = parse_position(pos_str);
  BitWord word = bitword_from_str(word_str);
  uint64_t bit = bitword_read(&word, position);

  printf("%" PRIu64 "\n", bit);

  bitword_destroy(&word);
  return EXIT_SUCCESS;
}

static int handle_bit_set(const char *pos_str, const char *word_str) {
  int position = parse_position(pos_str);
  BitWord word = bitword_from_str(word_str);
  char *result;

  bitword_set(&word, position);

  result = bitword_to_str(&word);
  printf("%s\n", result);

  free(result);
  bitword_destroy(&word);

  return EXIT_SUCCESS;
}

static int handle_bit_mod(const char *pos_str, const char *word_str) {
  int position = parse_position(pos_str);
  BitWord word = bitword_from_str(word_str);
  char *result;

  bitword_mod(&word, position);

  result = bitword_to_str(&word);
  printf("%s\n", result);

  free(result);
  bitword_destroy(&word);

  return EXIT_SUCCESS;
}

static int handle_error(const char *rate_str, const char *word_str) {
  double rate = parse_rate(rate_str);
  BitWord word = bitword_from_str(word_str);
  char *result;

  error_apply(&word, rate);

  result = bitword_to_str(&word);
  printf("%s\n", result);

  free(result);
  bitword_destroy(&word);

  return EXIT_SUCCESS;
}

static int handle_code(const char *poly_str, const char *data_str) {
  Polynomial polynomial = parse_polynomial(poly_str);
  char *padded_data_str = pad_binary_string_right(data_str, CODEC_DATA_BITS);
  BitWord data = bitword_from_str(padded_data_str);
  BitWord codeword = codec_encode_block(&data, polynomial);
  char *result = bitword_to_str(&codeword);

  printf("%s\n", result);

  free(result);
  bitword_destroy(&codeword);
  bitword_destroy(&data);
  free(padded_data_str);

  return EXIT_SUCCESS;
}

static int handle_decode(const char *poly_str, const char *codeword_str) {
  (void)parse_polynomial(poly_str);

  if (!is_binary_string(codeword_str) ||
      strlen(codeword_str) != CODEC_CODE_BITS) {
    fprintf(stderr, "Erreur: mot de code attendu sur 256 bits.\n");
    return EXIT_FAILURE;
  }

  BitWord codeword = bitword_from_str(codeword_str);
  BitWord data = codec_decode_block(&codeword);
  char *result = bitword_to_str(&data);

  printf("%s\n", result);

  free(result);
  bitword_destroy(&data);
  bitword_destroy(&codeword);

  return EXIT_SUCCESS;
}

static int handle_corrige(const char *poly_str, const char *codeword_str) {
  Polynomial polynomial = parse_polynomial(poly_str);
  GeneratorPolynomial generator = get_generator_from_poly(polynomial);
  BitWord codeword;
  char *result;
  int status;

  if (!is_binary_string(codeword_str) ||
      strlen(codeword_str) != CODEC_CODE_BITS) {
    fprintf(stderr, "Erreur: mot de code attendu sur 256 bits.\n");
    return EXIT_FAILURE;
  }

  codeword = bitword_from_str(codeword_str);
  status = codec_correct_block(&codeword, &generator);

  if (status < 0) {
    fprintf(stderr, "Attention: erreur non corrigeable comme erreur simple.\n");
  }

  result = bitword_to_str(&codeword);
  printf("%s\n", result);

  free(result);
  bitword_destroy(&codeword);

  return status < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}

static int handle_file_mode(const char *filename, const char *rate_str) {
  double rate = parse_rate(rate_str);

  fileio_process(filename, rate);

  return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
  const char *command;

  srand((unsigned int)time(NULL));

  if (argc < 2) {
    print_usage(argv[0]);
    return EXIT_FAILURE;
  }

  command = argv[1];

  if (strcmp(command, "error") == 0) {
    if (argc != 4) {
      print_usage(argv[0]);
      return EXIT_FAILURE;
    }

    return handle_error(argv[2], argv[3]);
  }

  if (strcmp(command, "code") == 0) {
    if (argc != 4) {
      print_usage(argv[0]);
      return EXIT_FAILURE;
    }

    return handle_code(argv[2], argv[3]);
  }

  if (strcmp(command, "decode") == 0) {
    if (argc != 4) {
      print_usage(argv[0]);
      return EXIT_FAILURE;
    }

    return handle_decode(argv[2], argv[3]);
  }

  if (strcmp(command, "corrige") == 0) {
    if (argc != 4) {
      print_usage(argv[0]);
      return EXIT_FAILURE;
    }

    return handle_corrige(argv[2], argv[3]);
  }

  if (strcmp(command, "read") == 0) {
    if (argc != 4) {
      print_usage(argv[0]);
      return EXIT_FAILURE;
    }

    return handle_bit_read(argv[2], argv[3]);
  }

  if (strcmp(command, "set") == 0) {
    if (argc != 4) {
      print_usage(argv[0]);
      return EXIT_FAILURE;
    }

    return handle_bit_set(argv[2], argv[3]);
  }

  if (strcmp(command, "mod") == 0) {
    if (argc != 4) {
      print_usage(argv[0]);
      return EXIT_FAILURE;
    }

    return handle_bit_mod(argv[2], argv[3]);
  }

  if (strcmp(command, "test") == 0) {
    if (argc != 2) {
      print_usage(argv[0]);
      return EXIT_FAILURE;
    }

    return tests_run_all();
  }

  if (strcmp(command, "search4poly") == 0) {
    if (argc != 2) {
      print_usage(argv[0]);
      return EXIT_FAILURE;
    }

    return search4poly_main();
  }

  if (argc == 3) {
    return handle_file_mode(argv[1], argv[2]);
  }

  fprintf(stderr, "Erreur: commande inconnue ou mauvais nombre d'arguments.\n");
  print_usage(argv[0]);

  return EXIT_FAILURE;
}
