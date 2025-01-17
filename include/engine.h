#ifndef ENGINE_H
#define ENGINE_H

#include "bitboard.h"

/**
 * @brief Setup the engine, including precomputation of move lookup tables.
 *
 * @param rook_move_table: rook move lookup table (array)
 * @param bishop_move_table: bishop move lookup table (array) 
 */
void setup_engine(BITBOARD *rook_move_table, BITBOARD *bishop_move_table);

#endif // ENGINE_H
