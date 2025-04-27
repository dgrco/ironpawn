#include "bitboard.h"
#include "engine.h"
#include "magic_info.h"
#include "uci.h"
#include "utils.h"
#include <limits.h>

#define RANK_LEN 8
#define DEFAULT_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

void test_bitboards();

#define MAX_RESPONSE 1024
static char response[MAX_RESPONSE];

int main(int argc, char **argv) {
  if (argc == 2) {
    if (str_eq(argv[1], "debug")) {
      //
      // DEBUGGING
      // Bitboard test
      test_bitboards();
    } else if (str_eq(argv[1], "rook_magic")) {
      ChessBitboards chess_bitboards;
      bb_init_chess_boards(&chess_bitboards, DEFAULT_FEN);
      compute_and_export_magics(chess_bitboards.rook_blocker_masks, 14,
                                "rook-magics.out");
    } else if (str_eq(argv[1], "bishop_magic")) {
      ChessBitboards chess_bitboards;
      bb_init_chess_boards(&chess_bitboards, DEFAULT_FEN);
      compute_and_export_magics(chess_bitboards.bishop_blocker_masks, 14,
                                "bishop-magics.out");
    }
    return 0;
  }

  //
  // Engine Setup
  ChessBitboards chess_bitboards;
  bb_init_chess_boards(&chess_bitboards, DEFAULT_FEN);
  BITBOARD *rook_move_table[64], *bishop_move_table[64];
  MagicInfo magic_info = init_magic_info();
  engine_setup(&chess_bitboards, rook_move_table, bishop_move_table,
               &magic_info);
  chess_bitboards.rook_move_table = rook_move_table;
  chess_bitboards.bishop_move_table = bishop_move_table;

  //
  // Main Loop
  printf("IronPawn by Dante Grieco\n");
  while (1) {
    String input = str_create("");
    str_read_from_stdin(&input, 100);
    if (process_uci_command(&input, &chess_bitboards, &magic_info, response,
                            MAX_RESPONSE) == -1) {
      break;
    }
    printf("%s", response);
  }

  // Engine Cleanup
  engine_cleanup(rook_move_table, bishop_move_table);

  return 0;
}

/// Bitboard Testing
/**/
void test_bitboards() {
  ChessBitboards bbs;
  bb_init_chess_boards(&bbs, DEFAULT_FEN);
  printf("White pawns:\n");
  bb_print(bbs.white_pawns);
  printf("\n");

  printf("White bishops:\n");
  bb_print(bbs.white_bishops);
  printf("\n");

  printf("White knights:\n");
  bb_print(bbs.white_knights);
  printf("\n");

  printf("White rooks:\n");
  bb_print(bbs.white_rooks);
  printf("\n");

  printf("White queens:\n");
  bb_print(bbs.white_queens);
  printf("\n");

  printf("White king:\n");
  bb_print(bbs.white_king);
  printf("\n");

  printf("Black pawns:\n");
  bb_print(bbs.black_pawns);
  printf("\n");

  printf("Black bishops:\n");
  bb_print(bbs.black_bishops);
  printf("\n");

  printf("Black knights:\n");
  bb_print(bbs.black_knights);
  printf("\n");

  printf("Black rooks:\n");
  bb_print(bbs.black_rooks);
  printf("\n");

  printf("Black queens:\n");
  bb_print(bbs.black_queens);
  printf("\n");

  printf("Black king:\n");
  bb_print(bbs.black_king);
  printf("\n");

  printf("All white pieces:\n");
  bb_print(bbs.white_pieces);
  printf("\n");

  printf("All black pieces:\n");
  bb_print(bbs.black_pieces);
  printf("\n");

  printf("All pieces:\n");
  bb_print(bbs.all_pieces);
  printf("\n");

  printf("All empty squares:\n");
  bb_print(bbs.empty_squares);
  printf("\n");

  // Precomputation table tests
  int knight_pos = (4 * RANK_LEN) + 4;
  printf("Knight at position %d:\n", knight_pos);
  bb_pretty_print(bbs.knight_moves[knight_pos]);
  printf("\n");

  int king_pos = (4 * RANK_LEN) + 4;
  printf("King at position %d:\n", king_pos);
  bb_pretty_print(bbs.king_moves[king_pos]);
  printf("\n");

  int rook_pos = (4 * RANK_LEN) + 4;
  printf("Rook at position %d:\n", rook_pos);
  bb_pretty_print(bbs.rook_blocker_masks[rook_pos]);
  printf("\n");
}

/// End of Bitboard Testing
