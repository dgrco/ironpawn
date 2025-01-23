#ifndef BITBOARD_H
#define BITBOARD_H

#include <stdbool.h>
typedef unsigned long long BITBOARD;

enum PieceType {
  EMPTY,
  PAWN,
  BISHOP,
  KNIGHT,
  ROOK,
  QUEEN,
  KING
};

enum PieceColor {
  WHITE = 8,
  BLACK = 16
};

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
  BITBOARD rook_blocker_masks[64];
  BITBOARD bishop_blocker_masks[64];
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

ChessBitboards bb_init_chess_boards(char *board_str);

void set_bit(BITBOARD *bb, unsigned int pos);
void clear_bit(BITBOARD *bb, unsigned int pos);
void toggle_bit(BITBOARD *bb, unsigned int pos);
bool is_occupied(BITBOARD bb, unsigned int pos);

//
// Magic Bitboards
void compute_and_export_magics(BITBOARD *blocker_masks, const unsigned int MAX_INDICES, char *filename);

#endif // BITBOARD_H
