#include "engine.h"
#include "bitboard.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

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
