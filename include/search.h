#ifndef SEARCH_H
#define SEARCH_H

#include <stddef.h>
#include <stdint.h>

#define R 8u
#define N ((1u << R) - 1u)
#define K (N - R)

#define GENERATOR_CANDIDATE_COUNT (1u << (R - 1u))

/**
 * @typedef Polynomial
 * @brief Représente un polynôme binaire stocké sur 16 bits.
 *
 * Le bit i indique si le terme x^i est présent.
 * Par exemple, 0x11D représente :
 *
 * x^8 + x^4 + x^3 + x^2 + 1
 *
 * Un polynôme générateur de degré R utilise les bits 0 à R.
 */
typedef uint16_t Polynomial;

/**
 * @typedef Syndrome
 * @brief Représente le syndrome d'un mot reçu.
 *
 * Le syndrome est le reste de la division polynomiale par le polynôme
 * générateur. Ici, le syndrome tient sur R bits.
 *
 * Le type uint16_t est utilisé pour garder les calculs intermédiaires simples,
 * même si le syndrome final tient sur un octet.
 */
typedef uint16_t Syndrome;

/**
 * @struct GeneratorPolynomial
 * @brief Représente un polynôme générateur et sa table de syndromes.
 *
 * Le champ polynomial contient le polynôme générateur G(x).
 *
 * Le tableau syndromes contient les syndromes des erreurs simples :
 *
 * syndromes[i] = x^i mod G(x)
 *
 * Un polynôme générateur est valide pour corriger une erreur simple si tous
 * ces syndromes sont non nuls et deux à deux distincts.
 */
typedef struct {
  /** Polynôme générateur G(x). */
  Polynomial polynomial;

  /** Table des syndromes des erreurs simples x^0 à x^(N-1). */
  Syndrome syndromes[N];
} GeneratorPolynomial;

/**
 * @brief Cherche les polynômes générateurs valides.
 *
 * La fonction parcourt tous les polynômes candidats de degré R dont le terme
 * constant vaut 1. Pour chaque candidat, elle construit la table des syndromes
 * des erreurs simples et conserve uniquement les polynômes valides.
 *
 * Si generators vaut NULL ou si capacity vaut 0, la fonction compte seulement
 * les polynômes valides sans les stocker.
 *
 * @param generators Tableau dans lequel stocker les polynômes valides.
 * @param capacity Nombre maximal de polynômes stockables dans generators.
 *
 * @return Nombre total de polynômes générateurs valides trouvés.
 */
size_t search_valid_generator_polynomials(GeneratorPolynomial generators[],
                                          size_t capacity);

/**
 * @brief Retourne un polynôme générateur valide.
 *
 * La fonction retourne le premier polynôme générateur valide trouvé par la
 * recherche expérimentale.
 *
 * Si aucun polynôme valide n'est trouvé, le champ polynomial vaut 0.
 *
 * @return Un polynôme générateur valide avec sa table de syndromes.
 */
GeneratorPolynomial search_get_generator_polynomial(void);

/**
 * @brief Lance la recherche expérimentale de polynômes générateurs valides.
 *
 * La fonction affiche les dimensions du code, la liste des polynômes
 * générateurs valides, puis la table de syndromes du polynôme retenu.
 *
 * @return EXIT_SUCCESS si au moins un polynôme valide est trouvé,
 *         EXIT_FAILURE sinon.
 */
int search4poly_main(void);

#endif
