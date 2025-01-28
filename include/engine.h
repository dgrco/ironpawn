#ifndef ENGINE_H
#define ENGINE_H

#include "bitboard.h"
#include "magic_info.h"
#include "utils.h"
#include <stdint.h>

typedef struct {
  enum PieceType type;
  enum PieceColor color;
} Piece;

#define RANK_2_MASK (BITBOARD)0xFF << 8
#define RANK_7_MASK (BITBOARD)0xFF << 48

// Given a moves vector, get the "from" or "to" positions.
#define GET_FROM_POS(move) (move & ((1U << 6) - 1))
#define GET_TO_POS(move) (move & 0b111111000000) >> 6

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
 * @brief Perform cleanup on the engine. This includes freeing allocated
 * resources in the lookup tables.
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
/**
 * @brief Determine if a color is in check.
 *
 * @param bbs: An existing ChessBitboards object.
 * @param magic: An initialized MagicInfo object.
 * @param color: The color in question.
 * @return Will return true if the color in question is in check, and false
 * otherwise.
 */
bool engine_color_in_check(ChessBitboards *bbs, MagicInfo *magic_info,
                           enum PieceColor color);

/**
 * @brief Make a move and update all relevant bitboards in bbs.
 *
 * @param bbs: An existing ChessBitboards object.
 * @param from_pos: Where the piece to move originates.
 * @param to_pos: Where the piece to move lands.
 * @return The captured piece.
 */
Piece engine_move(ChessBitboards *bbs, unsigned int from_pos,
                  unsigned int to_pos);

/**
 * @brief Replace the piece that may have been captured.
 *
 * @param bbs: An existing ChessBitboards object.
 * @param captured: The captured piece to be re-placed back.
 * @param captured_pos: The position which the piece was captured at.
 */
void engine_undo_capture(ChessBitboards *bbs, Piece *captured,
                         unsigned int captured_pos);

/**
 * @brief Returns a String of the move in chess notation (i.e., e2e4)
 *
 * @param move: The MoveInfo value.
 * @return A String in chess notation. NOTE: this must be freed using
 * str_free().
 */
String move_info_to_chess_notation(MoveInfo move);

#endif // ENGINE_H
