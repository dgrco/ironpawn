#ifndef ENGINE_H
#define ENGINE_H

#include "bitboard.h"
#include "magic_info.h"

/**
 * @brief Setup the engine, including precomputation of move lookup tables.
 *
 * @param bbs: A ChessBitboards object that has been initialized using
 * bb_init_chess_boards()
 * @param rook_move_table: rook move lookup table (2D array)
 * @param bishop_move_table: bishop move lookup table (2D array)
 * @param magic_info: MagicInfo object that is initiated in init_magic_info().
 * @note the tables should be setup as BITBOARD *table[64];
 */
void engine_setup(ChessBitboards *bbs, BITBOARD **rook_move_table,
                  BITBOARD **bishop_move_table, MagicInfo *magic_info);

/**
 * @brief Perform cleanup on the engine. This includes freeing allocated resources
 * in the lookup tables.
 *
 * @param rook_move_table: rook move lookup table (2D array)
 * @param bishop_move_table: bishop move lookup table (2D array) 
 */
void engine_cleanup(BITBOARD **rook_move_table, BITBOARD **bishop_move_table);

#endif // ENGINE_H
