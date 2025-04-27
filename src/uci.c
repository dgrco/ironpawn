#include "uci.h"
#include "bitboard.h"
#include "engine.h"
#include "search.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

void handle_uci_init(char *response, const int MAX_RESPONSE) {
  snprintf(response, MAX_RESPONSE,
           "id name IronPawn\nid author Dante Grieco\nuciok\n");
}

void handle_position(Vec *tokens, ChessBitboards *bbs, char *response,
                     const int MAX_RESPONSE) {
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
    snprintf(response, MAX_RESPONSE, "Unknown command: %s\n",
             str_create_from_strvec(tokens).data);
    str_free(&board_str);
    vec_freeref(&moves);
    return;
  }
  bb_init_chess_boards(bbs, board_str.data);
  str_free(&board_str);
  vec_freeref(&moves);
}

void handle_go(Vec *tokens, ChessBitboards *bbs, MagicInfo *magic,
               char *response, const int MAX_RESPONSE) {
  size_t depth = 6;
  size_t movetime = ULONG_MAX;
  size_t wtime = ULONG_MAX;
  size_t btime = ULONG_MAX;
  enum PieceColor turn = WHITE;

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
  if ((i = vec_indexof(tokens, STRING, "turn")) != -1) {
    turn = strtoul(vec_get(tokens, i + 1), NULL, 10);
  }

  EvalResult eval_res = search(bbs, magic, depth, turn);
  String chess_not = move_info_to_chess_notation(eval_res.best_move);
  engine_move(bbs, GET_FROM_POS(eval_res.best_move), GET_TO_POS(eval_res.best_move)); // TODO: remove?
  snprintf(response, MAX_RESPONSE, "bestmove %s\n", chess_not.data);
  printf("Best Score: %d\n", eval_res.eval);
  str_free(&chess_not);
}

/**
 * @brief Process a UCI command from a String object.
 *
 * @param cmd: The UCI command as a String
 * @param bbs: An existing ChessBitboards reference.
 * @param magic: An existing MagicInfo reference.
 * @param response: The buffer to write the response.
 * @param MAX_RESPONSE: The max size of the response buffer.
 * @return Returns -1 to simulate a "break" if in a loop, returns 0 otherwise.
 */
int process_uci_command(String *cmd, ChessBitboards *bbs, MagicInfo *magic,
                        char *response, const int MAX_RESPONSE) {
  memset(response, 0, MAX_RESPONSE * sizeof(char));
  Vec tokens = str_split(cmd);
  char *first_token = (char *)vec_get(&tokens, 0);
  if (str_eq(first_token, "quit")) {
    vec_free(&tokens);
    str_free(cmd);
    return -1;
  } else if (str_eq(first_token, "uci")) {
    handle_uci_init(response, MAX_RESPONSE);
  } else if (str_eq(first_token, "position")) {
    handle_position(&tokens, bbs, response, MAX_RESPONSE);
  } else if (str_eq(first_token, "go")) {
    handle_go(&tokens, bbs, magic, response, MAX_RESPONSE);
  } else if (str_eq(first_token, "dbg_print_white")) {
    // NOTE: NOT FOR USE IN WASM
    bb_pretty_print(bbs->white_pieces);
  } else if (str_eq(first_token, "dbg_print_black")) {
    // NOTE: NOT FOR USE IN WASM
    bb_pretty_print(bbs->black_pieces);
  } else {
    snprintf(response, MAX_RESPONSE, "Unknown command: %s", cmd->data);
  }
  vec_free(&tokens);
  str_free(cmd);
  return 0;
}
