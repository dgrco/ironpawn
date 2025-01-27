#include "engine.h"
#include "bitboard.h"
#include "utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define RANK_2_MASK (BITBOARD) 0xFF << 8
#define RANK_7_MASK (BITBOARD) 0xFF << 48

/**
 * @brief Sets up a table.
 *
 * @param blockers: The pieces' blocker masks
 * @param table: The current table that is being setup.
 * @param i: The current square index that is being processed.
 * @param magics: The magic values for this table.
 * @param shifts: The shift values for this table.
 * @param DIRECTIONS: an array of 4 direction offset in {rank, file} (2) format.
 */
void __table_setup(BITBOARD *blockers, BITBOARD **table, unsigned int i,
                   const BITBOARD *MAGICS, const unsigned int *SHIFTS,
                   const int (*DIRECTIONS)[2]) {

  table[i] = (BITBOARD *)calloc(1ULL << (64 - SHIFTS[i]), sizeof(BITBOARD));

  // Go through each relevant occupancy board and compute legal moves
  BITBOARD blocker_mask = blockers[i]; // TODO: generalize
  const unsigned int NUM_SET_BITS = __builtin_popcountll(blocker_mask);
  const unsigned long long TOTAL_BIT_VARIANTS = 1ULL << NUM_SET_BITS;

  for (unsigned long long relevant_i = 0; relevant_i < TOTAL_BIT_VARIANTS;
       relevant_i++) {
    unsigned long long relevant_i_copy = relevant_i;
    BITBOARD blocker_mask_copy = blocker_mask;

    // Get relevant occupancy board
    BITBOARD relevant_occupancy = 0;
    while (relevant_i_copy != 0) {
      unsigned int lsb_pos = __builtin_ctzll(blocker_mask_copy);
      relevant_occupancy |= (relevant_i_copy & 1) << lsb_pos;
      assert(blocker_mask_copy != 0); // for safety
      blocker_mask_copy &= blocker_mask_copy - 1;
      relevant_i_copy >>= 1;
    }

    // Compute (pseudo) legal moves
    BITBOARD pseudo_legal_moves = 0;
    for (unsigned int entry_index = 0; entry_index < 4; entry_index++) {
      int rank = i / 8 + DIRECTIONS[entry_index][0];
      int file = i % 8 + DIRECTIONS[entry_index][1];

      while (rank >= 0 && rank < 8 && file >= 0 && file < 8) {
        unsigned long long pos = (1ULL << (rank * 8 + file));
        pseudo_legal_moves |= pos;
        if ((pos & relevant_occupancy) != 0) {
          break;
        }
        rank += DIRECTIONS[entry_index][0];
        file += DIRECTIONS[entry_index][1];
      }
    }

    // Use magic numbers to assign the moves to the correct precomputation
    // table entry. A "blocker" is the relevant occupancy board ANDed with the
    // blocker mask.
    // Get the index using the magic number and shift value.
    BITBOARD move_index = (relevant_occupancy * MAGICS[i]) >> SHIFTS[i];
    assert(table[i][move_index] == 0); // for testing
    table[i][move_index] = pseudo_legal_moves;
  }
}

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
                  BITBOARD **bishop_move_table, MagicInfo *magic_info) {
  //
  // Rook Table
  const int ROOK_DIRS[4][2] = {{-1, 0}, {0, -1}, {1, 0}, {0, 1}};
  for (unsigned int i = 0; i < 64; i++) {
    __table_setup(bbs->rook_blocker_masks, rook_move_table, i,
                  magic_info->ROOK_MAGICS, magic_info->ROOK_SHIFTS, ROOK_DIRS);
  }

  //
  // Bishop Table
  const int BISHOP_DIRS[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
  for (unsigned int i = 0; i < 64; i++) {
    __table_setup(bbs->bishop_blocker_masks, bishop_move_table, i,
                  magic_info->BISHOP_MAGICS, magic_info->BISHOP_SHIFTS,
                  BISHOP_DIRS);
  }
}

/**
 * @brief Helper for table cleanup; free all elements of a table.
 *
 * @param table: the table to cleanup
 */
void __table_cleanup(BITBOARD **table) {
  for (unsigned int i = 0; i < 64; i++) {
    if (table[i]) {
      free(table[i]);
      table[i] = NULL;
    }
  }
}

/**
 * @brief Perform cleanup on the engine. This includes freeing allocated
 * resources in the lookup tables.
 *
 * @param rook_move_table: rook move lookup table (2D array)
 * @param bishop_move_table: bishop move lookup table (2D array)
 */
void engine_cleanup(BITBOARD **rook_move_table, BITBOARD **bishop_move_table) {
  __table_cleanup(rook_move_table);
  __table_cleanup(bishop_move_table);
}

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
                                      enum PieceColor color) {
  Vec moves = vec_create();

  if (color == WHITE) {
    //
    // White

    // Knights
    BITBOARD w_knights_copy = bbs->white_knights;
    while (w_knights_copy) {
      unsigned int from_pos = POP_LSB(w_knights_copy);
      BITBOARD w_knight_moves_copy = bbs->knight_moves[from_pos];
      while (w_knight_moves_copy) {
        unsigned int to_pos = POP_LSB(w_knight_moves_copy);
        if ((1ULL << to_pos) & ~bbs->white_pieces) {
          MoveInfo *move = (MoveInfo *)malloc(1 * sizeof(MoveInfo));
          *move = from_pos | (to_pos << 6);
          vec_append(&moves, move);
        }
      }
    }

    // King
    BITBOARD w_king_copy = bbs->white_king;
    while (w_king_copy) {
      unsigned int from_pos = POP_LSB(w_king_copy);
      BITBOARD w_king_moves_copy = bbs->king_moves[from_pos];
      while (w_king_moves_copy) {
        unsigned int to_pos = POP_LSB(w_king_moves_copy);
        if ((1ULL << to_pos) & ~bbs->white_pieces) {
          MoveInfo *move = (MoveInfo *)malloc(1 * sizeof(MoveInfo));
          *move = from_pos | (to_pos << 6);
          vec_append(&moves, move);
        }
      }
    }

    // Rooks
    BITBOARD w_rook_copy = bbs->white_rooks;
    while (w_rook_copy) {
      unsigned int from_pos = POP_LSB(w_rook_copy);
      BITBOARD blocker = bbs->rook_blocker_masks[from_pos] & bbs->all_pieces;
      BITBOARD magic_index = (blocker * magic->ROOK_MAGICS[from_pos]) >>
                             magic->ROOK_SHIFTS[from_pos];
      BITBOARD w_rook_moves_copy = bbs->rook_move_table[from_pos][magic_index];
      while (w_rook_moves_copy) {
        unsigned int to_pos = POP_LSB(w_rook_moves_copy);
        if ((1ULL << to_pos) & ~bbs->white_pieces) {
          MoveInfo *move = (MoveInfo *)malloc(1 * sizeof(MoveInfo));
          *move = from_pos | (to_pos << 6);
          vec_append(&moves, move);
        }
      }
    }

    // Bishop
    BITBOARD w_bishop_copy = bbs->white_bishops;
    while (w_bishop_copy) {
      unsigned int from_pos = POP_LSB(w_bishop_copy);
      BITBOARD blocker = bbs->bishop_blocker_masks[from_pos] & bbs->all_pieces;
      BITBOARD magic_index = (blocker * magic->BISHOP_MAGICS[from_pos]) >>
                             magic->BISHOP_SHIFTS[from_pos];
      BITBOARD w_bishop_moves_copy =
          bbs->bishop_move_table[from_pos][magic_index];
      while (w_bishop_moves_copy) {
        unsigned int to_pos = POP_LSB(w_bishop_moves_copy);
        if ((1ULL << to_pos) & ~bbs->white_pieces) {
          MoveInfo *move = (MoveInfo *)malloc(1 * sizeof(MoveInfo));
          *move = from_pos | (to_pos << 6);
          vec_append(&moves, move);
        }
      }
    }

    // Queen
    BITBOARD w_queen_copy = bbs->white_queens;
    while (w_queen_copy) {
      unsigned int from_pos = POP_LSB(w_queen_copy);

      // Diagonals
      BITBOARD diag_blocker =
          bbs->bishop_blocker_masks[from_pos] & bbs->all_pieces;
      BITBOARD diag_magic_index =
          (diag_blocker * magic->BISHOP_MAGICS[from_pos]) >>
          magic->BISHOP_SHIFTS[from_pos];
      BITBOARD w_diag_moves_copy =
          bbs->bishop_move_table[from_pos][diag_magic_index];
      while (w_diag_moves_copy) {
        unsigned int to_pos = POP_LSB(w_diag_moves_copy);
        if ((1ULL << to_pos) & ~bbs->white_pieces) {
          MoveInfo *move = (MoveInfo *)malloc(1 * sizeof(MoveInfo));
          *move = from_pos | (to_pos << 6);
          vec_append(&moves, move);
        }
      }

      // Straights
      BITBOARD straight_blocker =
          bbs->rook_blocker_masks[from_pos] & bbs->all_pieces;
      BITBOARD straight_magic_index =
          (straight_blocker * magic->ROOK_MAGICS[from_pos]) >>
          magic->ROOK_SHIFTS[from_pos];
      BITBOARD w_straight_moves_copy =
          bbs->rook_move_table[from_pos][straight_magic_index];
      while (w_straight_moves_copy) {
        unsigned int to_pos = POP_LSB(w_straight_moves_copy);
        if ((1ULL << to_pos) & ~bbs->white_pieces) {
          MoveInfo *move = (MoveInfo *)malloc(1 * sizeof(MoveInfo));
          *move = from_pos | (to_pos << 6);
          vec_append(&moves, move);
        }
      }
    }

    // Pawns TODO: en passant
    BITBOARD single_push = (bbs->white_pawns << 8) & bbs->empty_squares;
    BITBOARD double_push = ((bbs->white_pawns & RANK_2_MASK) << 16) &
                           bbs->empty_squares & (bbs->empty_squares << 8);
    while (single_push) {
      unsigned int to_pos = POP_LSB(single_push);
      unsigned int from_pos = to_pos - 8;
      MoveInfo *move = (MoveInfo *)malloc(1 * sizeof(MoveInfo));
      *move = from_pos | (to_pos << 6);
      vec_append(&moves, move);
    }
    while (double_push) {
      unsigned int to_pos = POP_LSB(double_push);
      unsigned int from_pos = to_pos - 16;
      MoveInfo *move = (MoveInfo *)malloc(1 * sizeof(MoveInfo));
      *move = from_pos | (to_pos << 6);
      vec_append(&moves, move);
    }

    BITBOARD pawns_copy = bbs->white_pawns;
    while (pawns_copy) {
      unsigned int from_pos = POP_LSB(pawns_copy);
      BITBOARD capture_mask = bbs->white_pawn_captures[from_pos];
      BITBOARD possible_captures = bbs->black_pieces & capture_mask;
      while (possible_captures) {
        unsigned int to_pos = POP_LSB(possible_captures);
        MoveInfo *move = (MoveInfo *)malloc(1 * sizeof(MoveInfo));
        *move = from_pos | (to_pos << 6);
        vec_append(&moves, move);
      }
    }
  } else if (color == BLACK) {
    //
    // Black

    // Knights
    BITBOARD b_knights_copy = bbs->black_knights;
    while (b_knights_copy) {
      unsigned int from_pos = POP_LSB(b_knights_copy);
      BITBOARD b_knight_moves_copy = bbs->knight_moves[from_pos];
      while (b_knight_moves_copy) {
        unsigned int to_pos = POP_LSB(b_knight_moves_copy);
        if ((1ULL << to_pos) & ~bbs->black_pieces) {
          MoveInfo *move = (MoveInfo *)malloc(1 * sizeof(MoveInfo));
          *move = from_pos | (to_pos << 6);
          vec_append(&moves, move);
        }
      }
    }

    // King
    BITBOARD b_king_copy = bbs->black_king;
    while (b_king_copy) {
      unsigned int from_pos = POP_LSB(b_king_copy);
      BITBOARD b_king_moves_copy = bbs->king_moves[from_pos];
      while (b_king_moves_copy) {
        unsigned int to_pos = POP_LSB(b_king_moves_copy);
        if ((1ULL << to_pos) & ~bbs->black_pieces) {
          MoveInfo *move = (MoveInfo *)malloc(1 * sizeof(MoveInfo));
          *move = from_pos | (to_pos << 6);
          vec_append(&moves, move);
        }
      }
    }

    // Rooks
    BITBOARD b_rook_copy = bbs->black_rooks;
    while (b_rook_copy) {
      unsigned int from_pos = POP_LSB(b_rook_copy);
      BITBOARD blocker = bbs->rook_blocker_masks[from_pos] & bbs->all_pieces;
      BITBOARD magic_index = (blocker * magic->ROOK_MAGICS[from_pos]) >>
                             magic->ROOK_SHIFTS[from_pos];
      BITBOARD b_rook_moves_copy = bbs->rook_move_table[from_pos][magic_index];
      while (b_rook_moves_copy) {
        unsigned int to_pos = POP_LSB(b_rook_moves_copy);
        if ((1ULL << to_pos) & ~bbs->black_pieces) {
          MoveInfo *move = (MoveInfo *)malloc(1 * sizeof(MoveInfo));
          *move = from_pos | (to_pos << 6);
          vec_append(&moves, move);
        }
      }
    }

    // Bishop
    BITBOARD b_bishop_copy = bbs->black_bishops;
    while (b_bishop_copy) {
      unsigned int from_pos = POP_LSB(b_bishop_copy);
      BITBOARD blocker = bbs->bishop_blocker_masks[from_pos] & bbs->all_pieces;
      BITBOARD magic_index = (blocker * magic->BISHOP_MAGICS[from_pos]) >>
                             magic->BISHOP_SHIFTS[from_pos];
      BITBOARD b_bishop_moves_copy =
          bbs->bishop_move_table[from_pos][magic_index];
      while (b_bishop_moves_copy) {
        unsigned int to_pos = POP_LSB(b_bishop_moves_copy);
        if ((1ULL << to_pos) & ~bbs->black_pieces) {
          MoveInfo *move = (MoveInfo *)malloc(1 * sizeof(MoveInfo));
          *move = from_pos | (to_pos << 6);
          vec_append(&moves, move);
        }
      }
    }

    // Queen
    BITBOARD b_queen_copy = bbs->black_queens;
    while (b_queen_copy) {
      unsigned int from_pos = POP_LSB(b_queen_copy);

      // Diagonals
      BITBOARD diag_blocker =
          bbs->bishop_blocker_masks[from_pos] & bbs->all_pieces;
      BITBOARD diag_magic_index =
          (diag_blocker * magic->BISHOP_MAGICS[from_pos]) >>
          magic->BISHOP_SHIFTS[from_pos];
      BITBOARD b_diag_moves_copy =
          bbs->bishop_move_table[from_pos][diag_magic_index];
      while (b_diag_moves_copy) {
        unsigned int to_pos = POP_LSB(b_diag_moves_copy);
        if ((1ULL << to_pos) & ~bbs->black_pieces) {
          MoveInfo *move = (MoveInfo *)malloc(1 * sizeof(MoveInfo));
          *move = from_pos | (to_pos << 6);
          vec_append(&moves, move);
        }
      }

      // Straights
      BITBOARD straight_blocker =
          bbs->rook_blocker_masks[from_pos] & bbs->all_pieces;
      BITBOARD straight_magic_index =
          (straight_blocker * magic->ROOK_MAGICS[from_pos]) >>
          magic->ROOK_SHIFTS[from_pos];
      BITBOARD b_straight_moves_copy =
          bbs->rook_move_table[from_pos][straight_magic_index];
      while (b_straight_moves_copy) {
        unsigned int to_pos = POP_LSB(b_straight_moves_copy);
        if ((1ULL << to_pos) & ~bbs->black_pieces) {
          MoveInfo *move = (MoveInfo *)malloc(1 * sizeof(MoveInfo));
          *move = from_pos | (to_pos << 6);
          vec_append(&moves, move);
        }
      }
    }

    // Pawns TODO: en passant
    BITBOARD single_push = (bbs->black_pawns >> 8) & bbs->empty_squares;
    BITBOARD double_push = ((bbs->black_pawns & RANK_7_MASK) >> 16) &
                           bbs->empty_squares & (bbs->empty_squares >> 8);
    while (single_push) {
      unsigned int to_pos = POP_LSB(single_push);
      unsigned int from_pos = to_pos + 8;
      MoveInfo *move = (MoveInfo *)malloc(1 * sizeof(MoveInfo));
      *move = from_pos | (to_pos << 6);
      vec_append(&moves, move);
    }
    while (double_push) {
      unsigned int to_pos = POP_LSB(double_push);
      unsigned int from_pos = to_pos + 16;
      MoveInfo *move =  (MoveInfo *)malloc(1 * sizeof(MoveInfo));
      *move = from_pos | (to_pos << 6);
      vec_append(&moves, move);
    }

    BITBOARD pawns_copy = bbs->black_pawns;
    while (pawns_copy) {
      unsigned int from_pos = POP_LSB(pawns_copy);
      BITBOARD capture_mask = bbs->black_pawn_captures[from_pos];
      BITBOARD possible_captures = bbs->white_pieces & capture_mask;
      while (possible_captures) {
        unsigned int to_pos = POP_LSB(possible_captures);
        MoveInfo *move = (MoveInfo *)malloc(1 * sizeof(MoveInfo));
        *move = from_pos | (to_pos << 6);
        vec_append(&moves, move);
      }
    }
  }

  return moves;
}
