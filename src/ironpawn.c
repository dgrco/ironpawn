#include "bitboard.h"
#include "engine.h"
#include "magic_info.h"
#include "search.h"
#include "utils.h"
#include <limits.h>
#include <stdlib.h>

#define RANK_LEN 8
#define DEFAULT_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

void handle_uci();
void handle_position(Vec *tokens);
void handle_go(Vec *tokens, ChessBitboards *bbs, MagicInfo *magic);

void test_bitboards();

int main(int argc, char **argv) {
  if (argc == 2) {
    if (str_eq(argv[1], "debug")) {
      //
      // DEBUGGING
      // Bitboard test
      /*test_bitboards();*/

      // Magic bitboards
      /*ChessBitboards bbs = bb_init_chess_boards(DEFAULT_FEN);*/
      /*compute_and_export_magics(bbs.rook_blocker_masks, 20,
       * "rook-magics.out");*/
      /*compute_and_export_magics(bbs.bishop_blocker_masks, 20,
       * "bishop-magics.out");*/
    } else if (str_eq(argv[1], "rook_magic")) {
      ChessBitboards chess_bitboards = bb_init_chess_boards(DEFAULT_FEN);
      compute_and_export_magics(chess_bitboards.rook_blocker_masks, 14,
                                "rook-magics.out");
    } else if (str_eq(argv[1], "bishop_magic")) {
      ChessBitboards chess_bitboards = bb_init_chess_boards(DEFAULT_FEN);
      compute_and_export_magics(chess_bitboards.bishop_blocker_masks, 14,
                                "bishop-magics.out");
    }
    return 0;
  }

  //
  // Engine Setup
  ChessBitboards chess_bitboards = bb_init_chess_boards(DEFAULT_FEN);
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
    Vec tokens = str_split(&input);
    char *first_token = (char *)vec_get(&tokens, 0);
    if (str_eq(first_token, "quit")) {
      vec_free(&tokens);
      str_free(&input);
      break;
    } else if (str_eq(first_token, "uci")) {
      handle_uci();
    } else if (str_eq(first_token, "position")) {
      handle_position(&tokens);
    } else if (str_eq(first_token, "go")) {
      handle_go(&tokens, &chess_bitboards, &magic_info);
    } else if (str_eq(first_token, "print_white")) {
      bb_pretty_print(chess_bitboards.white_pieces);
    } else if (str_eq(first_token, "print_black")) {
      bb_pretty_print(chess_bitboards.black_pieces);
    } else {
      printf("Unknown command: %s", input.data);
    }
    vec_free(&tokens);
    str_free(&input);
  }

  // Engine Cleanup
  engine_cleanup(rook_move_table, bishop_move_table);

  return 0;
}

/// UCI

void handle_uci() {
  printf("id name IronPawn\nid author Dante Grieco\nuciok\n");
}

void handle_position(Vec *tokens) {
  String board_str = str_dead();
  Vec moves = vec_dead();
  if (tokens->len <= 1) {
    return;
  }
  if (str_eq(vec_get(tokens, 1), "startpos")) {
    board_str = str_create(DEFAULT_FEN);
    int moves_idx = vec_indexof(tokens, STRING, "moves");
    if (moves_idx != -1) {
      moves = vec_refsubvec(tokens, moves_idx + 1, -1);
    }
  } else if (str_eq(vec_get(tokens, 1), "fen")) {
    int fen_idx = vec_indexof(tokens, STRING, "fen");
    int moves_idx = vec_indexof(tokens, STRING, "moves");
    if (moves_idx != -1) {
      Vec fen = vec_refsubvec(tokens, fen_idx + 1, moves_idx);
      board_str = str_create_from_strvec(&fen);
      moves = vec_refsubvec(tokens, moves_idx + 1, -1);
    } else {
      Vec fen = vec_refsubvec(tokens, fen_idx + 1, -1);
      board_str = str_create_from_strvec(&fen);
    }
  } else {
    printf("Unknown command: %s\n", str_create_from_strvec(tokens).data);
    str_free(&board_str);
    vec_freeref(&moves);
    return;
  }
  str_free(&board_str);
  vec_freeref(&moves);
}

void handle_go(Vec *tokens, ChessBitboards *bbs, MagicInfo *magic) {
  size_t depth = 6;
  size_t movetime = ULONG_MAX;
  size_t wtime = ULONG_MAX;
  size_t btime = ULONG_MAX;

  int i;
  // NOTE: using strtoul can enable unexpected results if negative values are
  // passed.
  if ((i = vec_indexof(tokens, STRING, "depth")) != -1) {
    depth = strtoul(vec_get(tokens, i + 1), NULL, 10);
  }
  if ((i = vec_indexof(tokens, STRING, "movetime")) != -1) {
    movetime = strtoul(vec_get(tokens, i + 1), NULL, 10);
  }
  if ((i = vec_indexof(tokens, STRING, "wtime")) != -1) {
    wtime = strtoul(vec_get(tokens, i + 1), NULL, 10);
  }
  if ((i = vec_indexof(tokens, STRING, "btime")) != -1) {
    btime = strtoul(vec_get(tokens, i + 1), NULL, 10);
  }

  EvalResult eval_res = search(bbs, magic, depth, WHITE);
  String chess_not = move_info_to_chess_notation(eval_res.best_move);
  printf("bestmove %s\n", chess_not.data);
  printf("score: %d\n", eval_res.eval);
  str_free(&chess_not); 
}

/// End of UCI

/// Bitboard Testing

void test_bitboards() {
  ChessBitboards bbs = bb_init_chess_boards(DEFAULT_FEN);
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
