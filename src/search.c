/**
 * @file search.c
 * @brief Recherche expérimentale des polynômes générateurs.
 */

#include "search.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Multiplier un polynôme par x revient à décaler ses bits
 * d'un cran vers la gauche.
 */
static Polynomial multiply_by_x(Syndrome syndrome) {
  return (Polynomial)(syndrome << 1u);
}

/*
 * Si le reste dépasse le degré du polynôme générateur, on le réduit
 * modulo G(x). Dans F2, cette réduction se fait avec un XOR.
 */
static Syndrome reduce_mod_generator(Polynomial polynomial,
                                     const GeneratorPolynomial *generator) {
  Polynomial generator_leading_bit = (Polynomial)(1u << R);

  if ((polynomial & generator_leading_bit) != 0) {
    polynomial ^= generator->polynomial;
  }

  return (Syndrome)(polynomial & N);
}

/*
 * Pour corriger une erreur simple, on étudie le syndrome du mot reçu.
 *
 * Si C(x) est un mot de code valide et E(x) une erreur, alors le mot reçu est :
 *
 *   M(x) = C(x) + E(x)
 *
 * Comme C(x) est divisible par le polynôme générateur G(x), on a :
 *
 *   M(x) mod G(x) = E(x) mod G(x)
 *
 * Pour une erreur simple en position i :
 *
 *   E(x) = x^i
 *
 * On veut donc calculer les syndromes :
 *
 *   x^0 mod G(x), x^1 mod G(x), ..., x^(N-1) mod G(x)
 *
 * Plutôt que de refaire une division complète à chaque fois, on utilise :
 *
 *   x^(i+1) mod G(x) = x * (x^i mod G(x)) mod G(x)
 *
 * La fonction syndrome_next calcule donc le syndrome suivant à partir
 * du syndrome courant.
 */
static Syndrome syndrome_next(Syndrome current_syndrome,
                              const GeneratorPolynomial *generator) {
  Polynomial product_by_x = multiply_by_x(current_syndrome);

  return reduce_mod_generator(product_by_x, generator);
}

/*
 * Construit la table des syndromes du polynôme générateur.
 * La valeur de retour indique si les syndromes des erreurs unitaires
 * sont tous non nuls et deux à deux distincts.
 */
static int build_syndrome_table(GeneratorPolynomial *generator) {
  int seen_syndromes[N + 1] = {0};
  Syndrome current_syndrome = 1u;

  memset(generator->syndromes, 0, sizeof(generator->syndromes));

  for (size_t position = 0; position < N; position++) {
    if (current_syndrome == 0 || seen_syndromes[current_syndrome]) {
      return 0;
    }

    generator->syndromes[position] = current_syndrome;
    seen_syndromes[current_syndrome] = 1;

    current_syndrome = syndrome_next(current_syndrome, generator);
  }

  return 1;
}

/*
 * Passe au candidat suivant dans la plage testée.
 * On ajoute 2 pour garder le bit de poids faible à 1.
 */
static Polynomial candidate_next(Polynomial candidate) {
  return (Polynomial)(candidate + 2u);
}

size_t search_valid_generator_polynomials(GeneratorPolynomial generators[],
                                          size_t capacity) {
  Polynomial first_candidate = (Polynomial)((1u << R) | 1u);
  Polynomial candidate_limit = (Polynomial)(1u << (R + 1u));
  size_t valid_count = 0;

  for (Polynomial candidate = first_candidate; candidate < candidate_limit;
       candidate = candidate_next(candidate)) {
    GeneratorPolynomial generator = {.polynomial = candidate, .syndromes = {0}};
    int is_valid = build_syndrome_table(&generator);

    if (!is_valid) {
      continue;
    }

    if (generators != NULL && valid_count < capacity) {
      generators[valid_count] = generator;
    }

    valid_count++;
  }

  return valid_count;
}

GeneratorPolynomial search_get_generator_polynomial(void) {
  GeneratorPolynomial generator = {0};

  search_valid_generator_polynomials(&generator, 1u);

  return generator;
}

static void print_syndrome_table(const GeneratorPolynomial *generator) {
  printf("\nTable des syndromes pour G(x) = 0x%03X\n",
         (unsigned int)generator->polynomial);

  for (size_t position = 0; position < N; position++) {
    if (position % 16u == 0) {
      printf("pos %03zu :", position);
    }

    printf(" %02X", (unsigned int)generator->syndromes[position]);

    if (position % 16u == 15u || position + 1u == N) {
      printf("\n");
    }
  }
}

int search4poly_main(void) {
  GeneratorPolynomial generators[GENERATOR_CANDIDATE_COUNT];
  size_t valid_count =
      search_valid_generator_polynomials(generators, GENERATOR_CANDIDATE_COUNT);

  printf("Recherche de polynomes generateurs\n");
  printf("R = %u\n", R);
  printf("N = %u\n", N);
  printf("K = %u\n", K);
  printf("Rendement = %u/%u = %.2f %%\n\n", K, N,
         100.0 * (double)K / (double)N);

  printf("Candidats testes : polynomes impairs de 0x%03X a 0x%03X\n\n",
         (unsigned int)((1u << R) | 1u), (unsigned int)((1u << (R + 1u)) - 1u));

  printf("Polynomes generateurs valides :\n");

  for (size_t i = 0; i < valid_count; i++) {
    printf("0x%03X\n", (unsigned int)generators[i].polynomial);
  }

  printf("\nNombre de polynomes valides trouves : %zu\n", valid_count);

  if (valid_count == 0) {
    return EXIT_FAILURE;
  }

  printf("\nPolynome retenu : 0x%03X\n",
         (unsigned int)generators[0].polynomial);

  print_syndrome_table(&generators[0]);

  return EXIT_SUCCESS;
}
