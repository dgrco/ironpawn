#ifndef BITBOARD_H
#define BITBOARD_H

#include <stdbool.h>
typedef unsigned long long BITBOARD;

// TODO: port all relevant code to use this macro

/// Return the LSB index of a value and clear the LSB.
#define POP_LSB(value)                                                         \
  ({                                                                           \
    unsigned int lsb_index = __builtin_ctzll(value);                           \
    value &= value - 1;                                                        \
    lsb_index;                                                                 \
  })

enum PieceType { EMPTY, PAWN, BISHOP, KNIGHT, ROOK, QUEEN, KING };

enum PieceColor {
  NOCOLOR = 0,
  WHITE = 1,
  BLACK = -1,
}; // TODO: maybe use the values... I don't know yet

typedef struct {
  // White bitboards
  BITBOARD white_pawns;
  BITBOARD white_bishops;
  BITBOARD white_knights;
  BITBOARD white_rooks;
  BITBOARD white_queens;
  BITBOARD white_king;

  // Black bitboards
  BITBOARD black_pawns;
  BITBOARD black_bishops;
  BITBOARD black_knights;
  BITBOARD black_rooks;
  BITBOARD black_queens;
  BITBOARD black_king;

  // Cumulative bitboards
  BITBOARD white_pieces;
  BITBOARD black_pieces;
  BITBOARD all_pieces;
  BITBOARD empty_squares;

  // Precomputation tables
  BITBOARD knight_moves[64];
  BITBOARD king_moves[64];
  BITBOARD black_pawn_moves[64];
  BITBOARD rook_blocker_masks[64];
  BITBOARD bishop_blocker_masks[64];
  BITBOARD white_pawn_captures[64];
  BITBOARD black_pawn_captures[64];
  // To be initialized outside of the init function:
  BITBOARD **rook_move_table;   // Should point to a BITBOARD [64]
  BITBOARD **bishop_move_table; // Should point to a BITBOARD [64]
} ChessBitboards;

void bb_print(BITBOARD bb);
void bb_pretty_print(BITBOARD bb);

BITBOARD init_white_pawns(char *board_str);
BITBOARD init_white_bishops(char *board_str);
BITBOARD init_white_knights(char *board_str);
BITBOARD init_white_rooks(char *board_str);
BITBOARD init_white_queens(char *board_str);
BITBOARD init_white_king(char *board_str);

BITBOARD init_black_pawns(char *board_str);
BITBOARD init_black_bishops(char *board_str);
BITBOARD init_black_knights(char *board_str);
BITBOARD init_black_rooks(char *board_str);
BITBOARD init_black_queens(char *board_str);
BITBOARD init_black_king(char *board_str);

BITBOARD init_white_pieces(ChessBitboards *bbs);
BITBOARD init_black_pieces(ChessBitboards *bbs);
BITBOARD init_all_pieces(ChessBitboards *bbs);
BITBOARD init_empty_squares(ChessBitboards *bbs);

void bb_init_chess_boards(ChessBitboards *bbs, char *board_str);

void set_bit(BITBOARD *bb, unsigned int pos);
void clear_bit(BITBOARD *bb, unsigned int pos);
void toggle_bit(BITBOARD *bb, unsigned int pos);
bool is_occupied(BITBOARD bb, unsigned int pos);

//
// Magic Bitboards
void compute_and_export_magics(BITBOARD *blocker_masks,
                               const unsigned int MAX_INDICES, char *filename);

#endif // BITBOARD_H
