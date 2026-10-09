#ifndef FILEIO_H
#define FILEIO_H

/**
 * @file fileio.h
 * @brief Transmission simulée d'un fichier complet.
 */

/**
 * @brief Lit un fichier et le convertit en chaîne binaire.
 *
 * Chaque octet devient 8 caractères '0' ou '1', bit de poids fort en premier.
 *
 * @param filename Chemin du fichier à lire.
 *
 * @return Une chaîne nouvellement allouée, à libérer avec free().
 */
char *file_to_bin_string(const char *filename);

/**
 * @brief Simule la transmission d'un fichier sur un canal bruité.
 *
 * Étapes :
 * 1. lecture du fichier et découpage en blocs de 240 bits, le dernier bloc
 *    étant complété par des zéros ;
 * 2. encodage de chaque bloc en mot de code de 256 bits ;
 * 3. ajout d'erreurs sur les 255 positions actives, selon le taux donné ;
 * 4. correction puis décodage de chaque bloc ;
 * 5. suppression du remplissage et écriture du résultat sur la sortie
 *    standard.
 *
 * @param filename Chemin du fichier à transmettre.
 * @param rate Taux d'erreur binaire du canal, entre 0.0 et 1.0.
 */
void fileio_process(const char *filename, double rate);

#endif
