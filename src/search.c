#include "search.h"
#include "bitboard.h"
#include "engine.h"
#include "magic_info.h"
#include "utils.h"
#include <limits.h>

int __eval(ChessBitboards *bbs) {
  // TODO: make good
  return __builtin_popcountll(bbs->white_pieces) -
         __builtin_popcountll(bbs->black_pieces);
}

/**
 * @brief Standard minimax algorithm w/ alpha-beta pruning.
 *
 * @param bbs: An existing ChessBitboards object.
 * @param magic: An existing MagicInfo object.
 * @param depth: The number of half-moves to search.
 * @param turn: the color whose turn it is to move.
 * @param a: alpha (maximizer's best).
 * @param b: beta (minimizer's best).
 * @return An evaluation score of the best path.
 */
int __minimax(ChessBitboards *bbs, MagicInfo *magic, unsigned int depth,
              enum PieceColor turn, int a, int b) {
  if (depth == 0) {
    return __eval(bbs);
  }

  int best_eval = turn == WHITE ? INT_MIN : INT_MAX;

  Vec potential_moves = engine_generate_pseudolegal_moves(bbs, magic, turn);

  for (unsigned int i = 0; i < potential_moves.len; i++) {
    MoveInfo move = *(MoveInfo *)vec_get(&potential_moves, i);
    unsigned int from_pos = GET_FROM_POS(move);
    unsigned int to_pos = GET_TO_POS(move);
    Piece captured = engine_move(bbs, from_pos, to_pos); // make the move (simulate it)

    int eval =
        __minimax(bbs, magic, depth - 1, turn == WHITE ? BLACK : WHITE, a, b);

    if ((turn == WHITE && eval > best_eval) ||
        (turn == BLACK && eval < best_eval)) {
      if (!engine_color_in_check(bbs, magic, turn))
        best_eval = eval;
    }

    if (turn == WHITE) {
      a = a >= best_eval ? a : best_eval;
      if (a >= b) {
        // reset the move 
        engine_move(bbs, to_pos, from_pos);
        engine_undo_capture(bbs, &captured, to_pos);
        break;
      }
    } else if (turn == BLACK) {
      b = b <= best_eval ? b : best_eval;
      if (b <= a) {
        // reset the move 
        engine_move(bbs, to_pos, from_pos);
        engine_undo_capture(bbs, &captured, to_pos);
        break;
      }
    }

    // reset the move
    engine_move(bbs, to_pos, from_pos); 
    engine_undo_capture(bbs, &captured, to_pos);
  }

  vec_free(&potential_moves);

  return best_eval;
}

/**
 * @brief Perform a Minimax search of a certain depth.
 *
 * @param bbs: An existing ChessBitboards object.
 * @param magic: An existing MagicInfo object.
 * @param depth: The number of half-moves to search.
 * @param turn: the color whose turn it is to move.
 * @return An EvalResult with the best move and its evaluation value.
 * NOTE: White is the maximizing player; black is minimizing.
 */
EvalResult search(ChessBitboards *bbs, MagicInfo *magic, unsigned int depth,
                  enum PieceColor turn) {
  MoveInfo best_move = 0;
  int best_eval = turn == WHITE ? INT_MIN : INT_MAX;

  Vec potential_moves = engine_generate_pseudolegal_moves(bbs, magic, turn);

  for (unsigned int i = 0; i < potential_moves.len; i++) {
    MoveInfo move = *(MoveInfo *)vec_get(&potential_moves, i);
    unsigned int from_pos = GET_FROM_POS(move);
    unsigned int to_pos = GET_TO_POS(move);
    Piece captured = engine_move(bbs, from_pos, to_pos); // make the move (simulate it)

    int eval = __minimax(bbs, magic, depth - 1, turn == WHITE ? BLACK : WHITE,
                         INT_MIN, INT_MAX);

    if ((turn == WHITE && eval > best_eval) ||
        (turn == BLACK && eval < best_eval)) {
      if (!engine_color_in_check(bbs, magic, turn)) {
        best_eval = eval;
        best_move = from_pos | (to_pos << 6); // TODO: add other flags
      }
    }

    // reset the move
    engine_move(bbs, to_pos, from_pos); 
    engine_undo_capture(bbs, &captured, to_pos);
  }

  vec_free(&potential_moves);

  return (EvalResult){.best_move = best_move, .eval = best_eval};
}
