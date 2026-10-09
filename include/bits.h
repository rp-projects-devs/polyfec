#ifndef BITS_H
#define BITS_H

#include <stddef.h>
#include <stdint.h>

/**
 * @file bits.h
 * @brief Manipulation de mots binaires de taille quelconque.
 *
 * Ce module permet de manipuler des mots binaires représentés sous forme
 * de plusieurs blocs de 64 bits.
 *
 * Convention utilisée :
 * - la position 0 correspond au bit le plus à droite ;
 * - la position 1 correspond au bit juste à gauche ;
 * - etc.
 *
 * Exemple :
 * @code
 * mot = "101010"
 *
 * position 0 -> 0
 * position 1 -> 1
 * position 2 -> 0
 * position 3 -> 1
 * position 4 -> 0
 * position 5 -> 1
 * @endcode
 */

#define BIT_BLOCK_SIZE 64

/**
 * @struct BitWord
 * @brief Représente un mot binaire de taille quelconque.
 *
 * Le mot est stocké dans un tableau de blocs de 64 bits.
 * Le bloc 0 contient les bits de poids faible, donc les positions 0 à 63.
 */
typedef struct {
  /** Tableau de blocs de 64 bits. */
  uint64_t *blocks;

  /** Nombre exact de bits du mot binaire. */
  size_t bit_len;

  /** Nombre de blocs de 64 bits utilisés. */
  size_t block_count;
} BitWord;

/**
 * @brief Lit un bit dans un bloc de 64 bits.
 *
 * @param position Position du bit à lire, entre 0 et 63.
 * @param bits Bloc de 64 bits.
 *
 * @return 0 si le bit vaut 0, 1 si le bit vaut 1.
 *
 * @note En cas de position invalide, le programme affiche une erreur
 * sur stderr puis s'arrête avec exit(EXIT_FAILURE).
 */
uint64_t bit_block_read(int position, uint64_t bits);

/**
 * @brief Met à 1 un bit dans un bloc de 64 bits.
 *
 * @param position Position du bit à modifier, entre 0 et 63.
 * @param bits Bloc de 64 bits.
 *
 * @return Le bloc modifié.
 *
 * @note En cas de position invalide, le programme affiche une erreur
 * sur stderr puis s'arrête avec exit(EXIT_FAILURE).
 */
uint64_t bit_block_set(int position, uint64_t bits);

/**
 * @brief Inverse un bit dans un bloc de 64 bits.
 *
 * @param position Position du bit à inverser, entre 0 et 63.
 * @param bits Bloc de 64 bits.
 *
 * @return Le bloc modifié.
 *
 * @note En cas de position invalide, le programme affiche une erreur
 * sur stderr puis s'arrête avec exit(EXIT_FAILURE).
 */
uint64_t bit_block_mod(int position, uint64_t bits);

/**
 * @brief Crée un mot binaire à partir d'une chaîne de caractères.
 *
 * La chaîne doit contenir uniquement les caractères '0' et '1'.
 *
 * @param s Chaîne binaire à convertir.
 *
 * @return Un BitWord nouvellement créé.
 *
 * @warning La mémoire allouée dans le BitWord doit être libérée avec
 * bitword_destroy().
 *
 * @note En cas de chaîne invalide ou d'échec d'allocation, le programme
 * affiche une erreur sur stderr puis s'arrête avec exit(EXIT_FAILURE).
 */
BitWord bitword_from_str(const char *s);

/**
 * @brief Libère la mémoire utilisée par un BitWord.
 *
 * Après l'appel, le mot est remis dans un état vide :
 * - blocks vaut NULL ;
 * - bit_len vaut 0 ;
 * - block_count vaut 0.
 *
 * @param word Mot binaire à libérer.
 */
void bitword_destroy(BitWord *word);

/**
 * @brief Lit un bit dans un mot binaire.
 *
 * @param word Mot binaire à lire.
 * @param position Position du bit à lire.
 *
 * @return 0 si le bit vaut 0, 1 si le bit vaut 1.
 *
 * @note En cas de mot invalide ou de position invalide, le programme
 * affiche une erreur sur stderr puis s'arrête avec exit(EXIT_FAILURE).
 */
uint64_t bitword_read(const BitWord *word, int position);

/**
 * @brief Met à 1 un bit dans un mot binaire.
 *
 * @param word Mot binaire à modifier.
 * @param position Position du bit à mettre à 1.
 *
 * @note En cas de mot invalide ou de position invalide, le programme
 * affiche une erreur sur stderr puis s'arrête avec exit(EXIT_FAILURE).
 */
void bitword_set(BitWord *word, int position);

/**
 * @brief Inverse un bit dans un mot binaire.
 *
 * Si le bit vaut 0, il devient 1.
 * Si le bit vaut 1, il devient 0.
 *
 * @param word Mot binaire à modifier.
 * @param position Position du bit à inverser.
 *
 * @note En cas de mot invalide ou de position invalide, le programme
 * affiche une erreur sur stderr puis s'arrête avec exit(EXIT_FAILURE).
 */
void bitword_mod(BitWord *word, int position);

/**
 * @brief Convertit un BitWord en chaîne binaire.
 *
 * @param word Mot binaire à convertir.
 *
 * @return Une chaîne nouvellement allouée contenant le mot binaire.
 *
 * @warning La chaîne retournée doit être libérée avec free().
 *
 * @note En cas de mot invalide ou d'échec d'allocation, le programme
 * affiche une erreur sur stderr puis s'arrête avec exit(EXIT_FAILURE).
 */
char *bitword_to_str(const BitWord *word);

/**
 * @brief Crée un mot binaire rempli de zéros.
 *
 * La fonction alloue un BitWord de taille bit_len. Tous les bits sont
 * initialisés à 0.
 *
 * @param bit_len Nombre de bits du mot à créer.
 *
 * @return Un BitWord nouvellement alloué.
 *
 * @warning La mémoire allouée doit être libérée avec bitword_destroy().
 *
 * @note En cas de taille invalide ou d'échec d'allocation, le programme
 * affiche une erreur sur stderr puis s'arrête avec exit(EXIT_FAILURE).
 */
BitWord bitword_create_zero(size_t bit_len);

/**
 * @brief Crée une copie indépendante d'un mot binaire.
 *
 * La fonction alloue un nouveau BitWord contenant les mêmes bits que word.
 * La copie possède sa propre mémoire et peut donc être modifiée sans changer
 * le mot original.
 *
 * @param word Mot binaire à copier.
 *
 * @return Une copie indépendante de word.
 *
 * @warning La mémoire allouée doit être libérée avec bitword_destroy().
 *
 * @note En cas de mot invalide ou d'échec d'allocation, le programme
 * affiche une erreur sur stderr puis s'arrête avec exit(EXIT_FAILURE).
 */
BitWord bitword_clone(const BitWord *word);

/**
 * @brief Applique un XOR entre un mot binaire et une valeur décalée.
 *
 * La fonction applique l'opération :
 *
 * word = word XOR (value << shift)
 *
 * Seuls les value_bit_len bits de poids faible de value sont utilisés.
 * Cette fonction est utile pour la division polynomiale, où l'on doit
 * soustraire un polynôme aligné sur le terme dominant du mot courant.
 *
 * Dans F2, la soustraction est un XOR.
 *
 * @param word Mot binaire à modifier.
 * @param value Valeur à appliquer par XOR.
 * @param value_bit_len Nombre de bits significatifs de value.
 * @param shift Décalage à appliquer avant le XOR.
 *
 * @note En cas de paramètres invalides, le programme affiche une erreur
 * sur stderr puis s'arrête avec exit(EXIT_FAILURE).
 */
void bitword_xor_shifted_value(BitWord *word, uint64_t value,
                               size_t value_bit_len, size_t shift);

#endif
