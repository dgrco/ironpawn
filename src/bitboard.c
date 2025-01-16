#include "bitboard.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_ITERATION_STEPS 1

/// Print a Bitboard in binary representation
void bb_print(BITBOARD bb) {
  for (unsigned int i = 0; i < 64; i++) {
    printf("%d", (int)(bb >> (63 - i)) & 0b1);
  }
  printf("\n");
}

/// Print a pretty version of the bitboard (8x8 board)
/// where a 1 is mapped to "X" and a 0 is mapped to "."
void bb_pretty_print(BITBOARD bb) {
  for (unsigned int i = 0; i < 8; i++) {
    for (unsigned int j = 0; j < 8; j++) {
      printf("%c ", ((bb >> (63 - (i * 8 + j))) & 0b1) == 1 ? 'X' : '.');
    }
    printf("\n");
  }
  printf("\n");
}

/// General method for computing a Bitboard given a FEN string
BITBOARD compute_initial_from_piece_char(char piece, char *fen) {
  BITBOARD bb = 0;
  unsigned int curr = 0;
  for (unsigned int i = 0; fen[i] != ' '; i++) {
    if (fen[i] == '/') {
      continue;
    }
    if (fen[i] == piece) {
      bb |= (1ULL << (63 - curr));
    }
    if (isdigit(fen[i])) {
      int offset = fen[i] - '0';
      curr += offset;
    } else {
      curr += 1;
    }
  }
  return bb;
}

BITBOARD init_white_pawns(char *board_str) {
  return compute_initial_from_piece_char('P', board_str);
}

BITBOARD init_white_bishops(char *board_str) {
  return compute_initial_from_piece_char('B', board_str);
}

BITBOARD init_white_knights(char *board_str) {
  return compute_initial_from_piece_char('N', board_str);
}

BITBOARD init_white_rooks(char *board_str) {
  return compute_initial_from_piece_char('R', board_str);
}

BITBOARD init_white_queens(char *board_str) {
  return compute_initial_from_piece_char('Q', board_str);
}

BITBOARD init_white_king(char *board_str) {
  return compute_initial_from_piece_char('K', board_str);
}

BITBOARD init_black_pawns(char *board_str) {
  return compute_initial_from_piece_char('p', board_str);
}

BITBOARD init_black_bishops(char *board_str) {
  return compute_initial_from_piece_char('b', board_str);
}

BITBOARD init_black_knights(char *board_str) {
  return compute_initial_from_piece_char('n', board_str);
}

BITBOARD init_black_rooks(char *board_str) {
  return compute_initial_from_piece_char('r', board_str);
}

BITBOARD init_black_queens(char *board_str) {
  return compute_initial_from_piece_char('q', board_str);
}

BITBOARD init_black_king(char *board_str) {
  return compute_initial_from_piece_char('k', board_str);
}

BITBOARD init_white_pieces(ChessBitboards *bbs) {
  return (BITBOARD)(bbs->white_pawns | bbs->white_bishops | bbs->white_knights |
                    bbs->white_rooks | bbs->white_queens | bbs->white_king);
}

BITBOARD init_black_pieces(ChessBitboards *bbs) {
  return (BITBOARD)(bbs->black_pawns | bbs->black_bishops | bbs->black_knights |
                    bbs->black_rooks | bbs->black_queens | bbs->black_king);
}

BITBOARD init_all_pieces(ChessBitboards *bbs) {
  return (BITBOARD)(bbs->white_pieces | bbs->black_pieces);
}

BITBOARD init_empty_squares(ChessBitboards *bbs) {
  return (BITBOARD)(~bbs->all_pieces);
}

/// Get the bitboard for the moves of a knight at a position
BITBOARD __get_knight_move_bb(unsigned int pos) {
  BITBOARD moves = 0;

  int knight_offsets[8][2] = {{-2, -1}, {-2, 1}, {2, -1}, {2, 1},
                              {-1, -2}, {-1, 2}, {1, -2}, {1, 2}};

  int rank = pos / 8;
  int file = pos % 8;
  for (unsigned int i = 0; i < 8; i++) {
    int rank_adj = rank + knight_offsets[i][0];
    int file_adj = file + knight_offsets[i][1];
    if (rank_adj >= 0 && file_adj >= 0 && rank_adj < 8 && file_adj < 8) {
      moves |= 1ULL << (rank_adj * 8 + file_adj);
    }
  }

  return moves;
}

/// Get the bitboard for the moves of a knight at a position
BITBOARD __get_king_move_bb(unsigned int pos) {
  BITBOARD moves = 0;

  int king_offsets[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
                            {0, 1},   {1, -1}, {1, 0},  {1, 1}};

  int rank = pos / 8;
  int file = pos % 8;
  for (unsigned int i = 0; i < 8; i++) {
    int rank_adj = rank + king_offsets[i][0];
    int file_adj = file + king_offsets[i][1];
    if (rank_adj >= 0 && file_adj >= 0 && rank_adj < 8 && file_adj < 8) {
      moves |= 1ULL << (rank_adj * 8 + file_adj);
    }
  }

  return moves;
}

/// Get blocking ray (rook) mask at a position
BITBOARD __get_blocking_ray_mask(unsigned int pos) {
  BITBOARD mask = 0;
  int ray_dirs[4][2] = {{-1, 0}, {0, 1}, {0, -1}, {1, 0}};

  for (unsigned int i = 0; i < 4; i++) {
    int rank = pos / 8 + ray_dirs[i][0];
    int file = pos % 8 + ray_dirs[i][1];

    while (rank >= 0 && rank < 8 && file >= 0 && file < 8) {
      mask |= 1ULL << (rank * 8 + file);
      rank += ray_dirs[i][0];
      file += ray_dirs[i][1];

      // Remove the last position since it cannot be a "blocker"
      int next_rank = rank + ray_dirs[i][0];
      int next_file = file + ray_dirs[i][1];
      if (next_rank < 0 || next_rank > 7 || next_file < 0 || next_file > 7) {
        break;
      }
    }
  }
  return mask;
}

/// Get blocking diagonal (bishop) mask at a position
BITBOARD __get_blocking_diag_mask(unsigned int pos) {
  BITBOARD mask = 0;
  int diag_dirs[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

  for (unsigned int i = 0; i < 4; i++) {
    int rank = pos / 8 + diag_dirs[i][0];
    int file = pos % 8 + diag_dirs[i][1];

    while (rank >= 0 && rank < 8 && file >= 0 && file < 8) {
      mask |= 1ULL << (rank * 8 + file);
      rank += diag_dirs[i][0];
      file += diag_dirs[i][1];

      // Remove the last position since it cannot be a "blocker"
      int next_rank = rank + diag_dirs[i][0];
      int next_file = file + diag_dirs[i][1];
      if (next_rank < 0 || next_rank > 7 || next_file < 0 || next_file > 7) {
        break;
      }
    }
  }
  return mask;
}

/// Initialize the ChessBitboards structure.
ChessBitboards bb_init_chess_boards(char *board_str) {
  ChessBitboards chess_bbs = {
      .white_pawns = init_white_pawns(board_str),
      .white_bishops = init_white_bishops(board_str),
      .white_knights = init_white_knights(board_str),
      .white_rooks = init_white_rooks(board_str),
      .white_queens = init_white_queens(board_str),
      .white_king = init_white_king(board_str),

      .black_pawns = init_black_pawns(board_str),
      .black_bishops = init_black_bishops(board_str),
      .black_knights = init_black_knights(board_str),
      .black_rooks = init_black_rooks(board_str),
      .black_queens = init_black_queens(board_str),
      .black_king = init_black_king(board_str),
  };

  // Cumulative bitboards
  chess_bbs.white_pieces = init_white_pieces(&chess_bbs);
  chess_bbs.black_pieces = init_black_pieces(&chess_bbs);
  chess_bbs.all_pieces = init_all_pieces(&chess_bbs);
  chess_bbs.empty_squares = init_empty_squares(&chess_bbs);

  // Precomputation tables
  for (unsigned int i = 0; i < 64; i++) {
    chess_bbs.knight_moves[i] = __get_knight_move_bb(i);
  }

  for (unsigned int i = 0; i < 64; i++) {
    chess_bbs.king_moves[i] = __get_king_move_bb(i);
  }

  for (unsigned int i = 0; i < 64; i++) {
    chess_bbs.rook_blocker_masks[i] = __get_blocking_ray_mask(i);
    chess_bbs.bishop_blocker_masks[i] = __get_blocking_diag_mask(i);
  }

  return chess_bbs;
}

/// Set a bit to 1. NOTE: a1 is index 0, h8 is index 63.
void set_bit(BITBOARD *bb, unsigned int pos) { *bb |= (1ULL << pos); }

/// Clear a bit to 0. NOTE: a1 is index 0, h8 is index 63.
void clear_bit(BITBOARD *bb, unsigned int pos) { *bb &= !(1ULL << pos); }

/// Toggle a bit. NOTE: a1 is index 0, h8 is index 63.
void toggle_bit(BITBOARD *bb, unsigned int pos) { *bb ^= (1ULL << pos); }

/// Check if a bit is set to 1 at a given position. NOTE: a1 is index 0, h8 is
/// index 63.
bool is_occupied(BITBOARD bb, unsigned int pos) {
  return (bb & (1ULL << pos)) != 0;
}

//
// Magic Bitboards

/// Generate random unsigned long long value.
unsigned long long __ull_rand() {
  unsigned long long rand_val = 0;
  for (unsigned int i = 0; i < 64; i += 32) {
    rand_val |= (unsigned long long)(rand()) << i;
  }
  return rand_val;
}

typedef struct {
  unsigned int first;
  unsigned int second;
} UInt_Tuple;

/// Returns a tuple where the first value is the
/// index of the least-significant bit in a BITBOARD (i.e, in a mask),
/// and the second value is the result of 'val' being right-shifted the minimum
/// number of times until the lsb is popped.
UInt_Tuple __pop_lsb(BITBOARD val) {
  UInt_Tuple ret;
  unsigned int idx = 0;

  while ((val & 1) == 0) {
    val >>= 1;
    idx++;
  }
  val >>= 1;
  idx++;

  ret.first = idx;
  ret.second = val;

  return ret;
}

typedef struct {
  BITBOARD *data;
  unsigned int size;
} DynArray;

void compute_and_export_magics(BITBOARD *blocker_masks,
                               unsigned int max_num_indices, char *filename) {
  // Magic number management
  BITBOARD magics[64] = {0};
  BITBOARD max_sizes[64];
  memset(max_sizes, UINT64_MAX, 64 * sizeof(BITBOARD));
  unsigned int max_bit_shifts[64] = {0};

  unsigned long long iterated_squares = 0;
  unsigned long long iterations = 0;
  unsigned int min_bits_found = 64;

  // Setup blocker boards.
  // blocker_board = blocker_mask & occupancy
  DynArray blocker_boards[64];
  for (unsigned int i = 0; i < 64; i++) {
    // Set up arrays that hold the indices of each set bit in this blocker mask
    unsigned int set_bit_indices[64] = {
        0}; // INFO: 64 is the max, much fewer will actually be used

    BITBOARD curr_mask = blocker_masks[i];

    unsigned int num_set_indices = 0, index = 0;
    while (curr_mask != 0) {
      if ((curr_mask & 0b1) == 1) {
        set_bit_indices[num_set_indices] = index;
        num_set_indices++;
      }
      curr_mask >>= 1;
      index++;
    }

    // Initialize the blocker boards
    BITBOARD num_boards = 1ULL << num_set_indices;
    blocker_boards[i].data = (BITBOARD *)malloc(num_boards * sizeof(BITBOARD));
    blocker_boards[i].size = num_boards;

    // Use the above array to perform a variant of binary counting
    // where each bit of the count is shifted according to the
    // currently iterated index in the array
    for (unsigned int j = 0; j < num_boards; j++) {
      index = 0;
      unsigned int count = j;
      do {
        blocker_boards[i].data[j] |=
            ((unsigned long long)(count & 0b1) << set_bit_indices[index]);
        index++;
        count >>= 1;
      } while (count != 0);
    }
  }

  // Find the magic numbers.
  char in[2] = {0};
  while (1) {
    // Output handling
    if (iterated_squares != 0 && iterations % MAX_ITERATION_STEPS == 0) {
      printf("\n");
      printf("Would you like to terminate and output the magics? If no, "
             "another iteration of all the squares will be performed. (y/n): ");
      fgets(in, 2, stdin);
      in[1] = 0;
      if (strncmp(in, "y", 1) == 0 || strncmp(in, "Y", 1) == 0) {
        FILE *fp = fopen(filename, "w");
        if (!fp) {
          fprintf(stderr, "Unable to write to file %s\n", filename);
          exit(1);
        }

        fprintf(fp, "magics = {");
        for (unsigned int i = 0; i < 64; i++) {
          fprintf(fp, "%llu%s", magics[i], i == 63 ? "}\n" : ", ");
        }
        fprintf(fp, "shifts = {");
        for (unsigned int i = 0; i < 64; i++) {
          fprintf(fp, "%u%s", max_bit_shifts[i], i == 63 ? "}\n" : ", ");
        }
        fprintf(fp, "sizes = {");
        for (unsigned int i = 0; i < 64; i++) {
          fprintf(fp, "%llu%s", max_sizes[i], i == 63 ? "}\n" : ", ");
        }

        fclose(fp);
        break;
      } // Anything other than "y/Y" is considered as "No".
    }

    srand(time(NULL));
    for (int i = 0; i < 64; i++) {
      BITBOARD magic = __ull_rand();
      bool skip_flag = false;

      unsigned int bit_shifts = max_bit_shifts[i] == 0 ? 64 - max_num_indices : max_bit_shifts[i];

      bool unique_magic_result[1ULL << (64 - bit_shifts)];
      memset(unique_magic_result, 0, (1ULL << (64 - bit_shifts)) * sizeof(_Bool));

      BITBOARD max_magic_applied = 0;

      for (unsigned int j = 0; j < blocker_boards[i].size; j++) {
        BITBOARD blocker = blocker_boards[i].data[j];
        BITBOARD magic_applied = (blocker * magic) >> bit_shifts;

        if (magic_applied > max_magic_applied) {
          max_magic_applied = magic_applied;
        }

        if (unique_magic_result[magic_applied]) {
          skip_flag = true;
          break;
        } else {
          unique_magic_result[magic_applied] = true;
        }
      }

      if (max_sizes[i] <= max_magic_applied) {
        skip_flag = true;
      }

      if (skip_flag) {
        // re-compute a magic number for this square
        i--;
        continue;
      }

      // Update the max size for this square
      max_sizes[i] = max_magic_applied;

      // Set and update bit shift values
      unsigned int index = 0;
      while (max_magic_applied != 0) {
        if ((max_magic_applied & 1) == 1) {
          max_bit_shifts[i] = 64 - index;
        }
        index++;
        max_magic_applied >>= 1;
      }
      
      if (64 - max_bit_shifts[i] < min_bits_found) {
        min_bits_found = 64 - max_bit_shifts[i];
      }

      // Update the magic value to this improved number
      magics[i] = magic;

      // Output progress
      printf("\033[2J"); // ANSI escape code to clear the screen
      printf("\033[H");  // ANSI escape code to move cursor to top-left
      printf("Iteration: %llu / %d\n", iterations % MAX_ITERATION_STEPS + 1,
             MAX_ITERATION_STEPS);
      printf("Squares computed: %llu / 64\n", iterated_squares % 64 + 1);
      unsigned int size_sum = 0;
      for (unsigned int i = 0; i < 64; i++) {
        size_sum += max_sizes[i];
      }
      printf("Total size: %0.2f kB\n", size_sum / 1000.0f);
      printf("Average Size Per Square: %0.2f kB\n", (size_sum / 64.0f) / 1000.0f);
      printf("Lowest Required Bit Count: %u\n", min_bits_found);
      fflush(stdout);
      iterated_squares++;
    }
    iterations++;
  }

  printf("\n");

  // Free heap-allocated objects
  for (unsigned int i = 0; i < 64; i++) {
    if (blocker_boards[i].data) {
      free(blocker_boards[i].data);
      blocker_boards[i].data = NULL;
    }
  }
}
