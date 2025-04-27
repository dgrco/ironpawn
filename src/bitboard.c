#include "bitboard.h"
#include <ctype.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FILE_A 0x4040404040404040
#define FILE_H 0x0101010101010101

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
      printf("%c ", ((bb >> (63 - (i * 8 + j))) & 0b1) == 1 ? 'x' : '.');
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

/**
 * @brief Obtain pawn capture masks for a given square position.
 *
 * @param pos: square position (pos 0 -> h1, pos 63 -> a8)
 * @param capture_offsets: a list of 2 elements giving the offsets for pawn
 * captures. NOTE: the format should be {file_a_facing_offset, file_h_facing_offset}.
 * TODO: change the param to a struct for clarity.
 *
 * @return The capture mask for the given square position.
 */
BITBOARD __get_pawn_capture_mask(unsigned int pos, int capture_offsets[2]) {
  BITBOARD capture_mask = 0;

  // WARN: is casting important here?
  capture_mask |= (1ULL << (unsigned long long)(pos + capture_offsets[0])) & ~FILE_A;
  capture_mask |= (1ULL << (unsigned long long)(pos + capture_offsets[1])) & ~FILE_H;

  return capture_mask;
}

/// Initialize the ChessBitboards structure.
void bb_init_chess_boards(ChessBitboards *bbs, char *board_str) {
  bbs->white_pawns = init_white_pawns(board_str);
  bbs->white_bishops = init_white_bishops(board_str);
  bbs->white_knights = init_white_knights(board_str);
  bbs->white_rooks = init_white_rooks(board_str);
  bbs->white_queens = init_white_queens(board_str);
  bbs->white_king = init_white_king(board_str);

  bbs->black_pawns = init_black_pawns(board_str);
  bbs->black_bishops = init_black_bishops(board_str);
  bbs->black_knights = init_black_knights(board_str);
  bbs->black_rooks = init_black_rooks(board_str);
  bbs->black_queens = init_black_queens(board_str);
  bbs->black_king = init_black_king(board_str);

  //
  // Cumulative bitboards
  bbs->white_pieces = init_white_pieces(bbs);
  bbs->black_pieces = init_black_pieces(bbs);
  bbs->all_pieces = init_all_pieces(bbs);
  bbs->empty_squares = init_empty_squares(bbs);

  //
  // Precomputation tables

  // Knights
  for (unsigned int i = 0; i < 64; i++) {
    bbs->knight_moves[i] = __get_knight_move_bb(i);
  }

  // Kings
  for (unsigned int i = 0; i < 64; i++) {
    bbs->king_moves[i] = __get_king_move_bb(i);
  }

  // Pawns
  int white_capture_offsets[2] = {7, 9};
  int black_capture_offsets[2] = {-7, -9};
  for (unsigned int i = 0; i < 64; i++) {
    bbs->white_pawn_captures[i] =
        __get_pawn_capture_mask(i, white_capture_offsets);
    bbs->black_pawn_captures[i] =
        __get_pawn_capture_mask(i, black_capture_offsets);
  }

  // Get blocker masks for rooks/bishops... to be used in magic setup
  for (unsigned int i = 0; i < 64; i++) {
    bbs->rook_blocker_masks[i] = __get_blocking_ray_mask(i);
    bbs->bishop_blocker_masks[i] = __get_blocking_diag_mask(i);
  }
}

/// Set a bit to 1. NOTE: a1 is index 0, h8 is index 63.
void set_bit(BITBOARD *bb, unsigned int pos) { *bb |= (1ULL << pos); }

/// Clear a bit to 0. NOTE: a1 is index 0, h8 is index 63.
void clear_bit(BITBOARD *bb, unsigned int pos) { *bb &= ~(1ULL << pos); }

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

volatile sig_atomic_t terminate = 0;
void __signal_handler(int signo) {
  if (signo == SIGINT) {
    terminate = 1;
  }
}

void compute_and_export_magics(BITBOARD *blocker_masks,
                               const unsigned int MAX_INDICES, char *filename) {
  // Magic number management
  BITBOARD magics[64] = {0};
  BITBOARD max_sizes[64];
  memset(max_sizes, (BITBOARD)INT_MAX, 64 * sizeof(BITBOARD));
  unsigned int max_bits[64] = {0}; // the number of bits needed per square

  // Register the signal (Ctrl+C) to cleanup and force output magic output
  // file
  if (signal(SIGINT, __signal_handler) == SIG_ERR) {
    perror("signal");
    exit(1);
  }

  unsigned long long iterated_squares = 0;
  unsigned long long iterations = 0;
  unsigned int min_bits_found = 64;

  // Setup blocker boards.
  // occupancy_board = blocker_mask & occupancy
  DynArray occupancy_boards[64];
  memset(occupancy_boards, 0, 64 * sizeof(DynArray));
  for (unsigned int i = 0; i < 64; i++) {
    // Initialize the occupancy boards
    BITBOARD num_occupancies = 1ULL << __builtin_popcountll(blocker_masks[i]);
    if (!occupancy_boards[i].data) {
      occupancy_boards[i].data =
          (BITBOARD *)calloc(num_occupancies, sizeof(BITBOARD));
      occupancy_boards[i].size = num_occupancies;
    }

    // Use the above array to perform a variant of binary counting
    // where each bit of the count is shifted according to the
    // currently iterated index in the array
    for (unsigned int j = 0; j < num_occupancies; j++) {
      BITBOARD curr_mask = blocker_masks[i];
      unsigned long long j_cpy = j;
      while (j_cpy != 0) {
        unsigned int index = __builtin_ctzll(curr_mask); // get LSB index
        curr_mask &= curr_mask - 1;                      // clear LSB
        occupancy_boards[i].data[j] |= (j_cpy & 1) << index;
        j_cpy >>= 1;
      }
    }
  }

  // Find the magic numbers.
  srand(time(NULL));
  while (1) {
    if (terminate) {
      // Program terminated...
      break;
    }

    for (int i = 0; i < 64; i++) {
      if (terminate) {
        // Program terminated. Here is where the code will loop the majority of
        // the time so it is crucial to check for termination here.
        break;
      }

      BITBOARD magic = __ull_rand() & __ull_rand() & __ull_rand();
      bool skip_flag = false;

      // Use (bits - 1) since we are aiming to minimize bits
      unsigned int bit_shifts =
          max_bits[i] == 0 ? 64 - (MAX_INDICES - 1) : 64 - (max_bits[i] - 1);

      bool unique_magic_result[1ULL << (64 - bit_shifts)];
      memset(unique_magic_result, 0,
             (unsigned long long)((1ULL << (64 - bit_shifts)) * sizeof(_Bool)));

      BITBOARD max_magic_applied = 0;

      for (unsigned int j = 0; j < occupancy_boards[i].size; j++) {
        BITBOARD blocker = occupancy_boards[i].data[j];
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

      // Set and update bit values
      max_bits[i] = 64 - bit_shifts;

      if (max_bits[i] < min_bits_found) {
        min_bits_found = max_bits[i];
      }

      // Update the magic value to this improved number
      magics[i] = magic;

      // Output progress
      printf("\033[2J"); // ANSI escape code to clear the screen
      printf("\033[H");  // ANSI escape code to move cursor to top-left
      printf("Iteration: %llu\n", iterations);
      printf("Square: %llu / 64\n", iterated_squares % 64 + 1);
      unsigned int size_sum = 0;
      for (unsigned int i = 0; i < 64; i++) {
        size_sum += max_sizes[i];
      }
      printf("Total size: %0.2f kB\n", size_sum / 1000.0f);
      printf("Average Size Per Square: %0.2f kB\n",
             (size_sum / 64.0f) / 1000.0f);
      printf("Lowest Required Bit Count: %u\n", min_bits_found);
      printf("(press Ctrl+C to terminate and output the magics output file)\n");
      fflush(stdout);
      iterated_squares++;
    }

    iterations++;
  }

  printf("\n");

  FILE *fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Unable to write to file %s\n", filename);
    exit(1);
  }

  for (unsigned int i = 0; i < 64; i++) {
    fprintf(fp, "%llu%s", magics[i], i == 63 ? "\n" : " ");
  }
  for (unsigned int i = 0; i < 64; i++) {
    fprintf(fp, "%u%s", 64 - max_bits[i], i == 63 ? "\n" : " ");
  }

  fclose(fp);

  // Free heap-allocated objects
  for (unsigned int i = 0; i < 64; i++) {
    if (occupancy_boards[i].data) {
      free(occupancy_boards[i].data);
      occupancy_boards[i].data = NULL;
    }
  }
}
