#ifndef CODEC_H
#define CODEC_H

#include "bits.h"
#include "search.h"

#include <stddef.h>

/**
 * @file codec.h
 * @brief Encodage, correction et décodage d'un bloc.
 *
 * Organisation d'un mot de code de 256 bits (position 0 = bit de poids
 * faible) :
 *
 * @code
 * positions   0 à   7 : reste de la division par G(x) (redondance)
 * positions   8 à 247 : 240 bits de données (30 octets)
 * positions 248 à 254 : bits neutralisés, toujours à 0
 * position        255 : bit mort, hors du code (pas de syndrome associé)
 * @endcode
 *
 * Les positions 0 à 254 forment le code théorique de longueur N = 255 :
 * une erreur simple sur n'importe laquelle de ces positions est corrigée.
 */

/** Nombre de bits de données par bloc. */
#define CODEC_DATA_BITS 240u

/** Nombre de bits d'un mot de code (conteneur). */
#define CODEC_CODE_BITS 256u

/**
 * @brief Encode un bloc de 240 bits en mot de code de 256 bits.
 *
 * L'encodage est systématique : les données sont recopiées telles quelles à
 * partir de la position R, puis le reste de la division de data(x) * x^R par
 * G(x) est écrit dans les R bits de poids faible. Le mot obtenu est divisible
 * par G(x).
 *
 * @param data_block Bloc de données, exactement CODEC_DATA_BITS bits.
 * @param generator_poly Polynôme générateur G(x).
 *
 * @return Un mot de code nouvellement alloué, à libérer avec
 *         bitword_destroy().
 */
BitWord codec_encode_block(const BitWord *data_block,
                           Polynomial generator_poly);

/**
 * @brief Corrige une erreur simple dans un mot de code.
 *
 * Le syndrome du mot est recherché dans la table du générateur ; s'il y est,
 * le bit correspondant est inversé.
 *
 * @param codeword Mot de code de CODEC_CODE_BITS bits, modifié en place.
 * @param generator Polynôme générateur et sa table de syndromes.
 *
 * @return 1 si une correction a été appliquée, 0 si le syndrome est nul
 *         (aucune erreur détectée), -1 si le syndrome ne correspond à aucune
 *         erreur simple.
 */
int codec_correct_block(BitWord *codeword,
                        const GeneratorPolynomial *generator);

/**
 * @brief Extrait les 240 bits de données d'un mot de code.
 *
 * Les bits de redondance, les bits neutralisés et le bit mort sont ignorés.
 *
 * @param codeword Mot de code de CODEC_CODE_BITS bits.
 *
 * @return Le bloc de données, à libérer avec bitword_destroy().
 */
BitWord codec_decode_block(const BitWord *codeword);

#endif
