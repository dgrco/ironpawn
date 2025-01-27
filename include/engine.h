#ifndef ENGINE_H
#define ENGINE_H

#include "bitboard.h"
#include "magic_info.h"
#include "utils.h"
#include <stdint.h>

/**
 * @brief A 16-bit value where:
 * bits 0-5: The 'from' position (0-63).
 * bits 6-11: The 'to' position (0-63).
 * bits 12-15: TODO: special flags (en passant, etc.).
 */
typedef uint16_t MoveInfo;

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

/**
 * @brief Computes all pseudo-legal moves given the current board.
 *
 * @param bbs: An initialized ChessBitboards object.
 * @param magic: An initialized MagicInfo object.
 * @param color: The color to generate moves from.
 * @return A Vec of MoveInfo values.
 * @note This returned Vec must be freed using vec_free().
 */
Vec engine_generate_pseudolegal_moves(ChessBitboards *bbs, MagicInfo *magic,
                                      enum PieceColor color);

#endif // ENGINE_H
