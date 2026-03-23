#include "search.h"
#include "bitboard.h"
#include "engine.h"
#include "magic_info.h"
#include <limits.h>

//
// Position Tables

// White Pawns
const int white_pawn_table[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},         {50, 50, 50, 50, 50, 50, 50, 50},
    {10, 10, 20, 30, 30, 20, 10, 10}, {5, 5, 10, 25, 25, 10, 5, 5},
    {0, 0, 0, 20, 20, 0, 0, 0},       {5, -5, -10, 0, 0, -10, -5, 5},
    {5, 10, 10, -20, -20, 10, 10, 5}, {0, 0, 0, 0, 0, 0, 0, 0}};

// Black Pawns
const int black_pawn_table[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},         {5, 10, 10, -20, -20, 10, 10, 5},
    {5, -5, -10, 0, 0, -10, -5, 5},   {0, 0, 0, 20, 20, 0, 0, 0},
    {5, 5, 10, 25, 25, 10, 5, 5},     {10, 10, 20, 30, 30, 20, 10, 10},
    {50, 50, 50, 50, 50, 50, 50, 50}, {0, 0, 0, 0, 0, 0, 0, 0}};

// White Knights
const int white_knight_table[8][8] = {{-50, -40, -30, -30, -30, -30, -40, -50},
                                      {-40, -20, 0, 0, 0, 0, -20, -40},
                                      {-30, 0, 10, 15, 15, 10, 0, -30},
                                      {-30, 5, 15, 20, 20, 15, 5, -30},
                                      {-30, 0, 15, 20, 20, 15, 0, -30},
                                      {-30, 5, 10, 15, 15, 10, 5, -30},
                                      {-40, -20, 0, 5, 5, 0, -20, -40},
                                      {-50, -40, -30, -30, -30, -30, -40, -50}};

// Black Knights
const int black_knight_table[8][8] = {{-50, -40, -30, -30, -30, -30, -40, -50},
                                      {-40, -20, 0, 5, 5, 0, -20, -40},
                                      {-30, 5, 10, 15, 15, 10, 5, -30},
                                      {-30, 0, 15, 20, 20, 15, 0, -30},
                                      {-30, 5, 15, 20, 20, 15, 5, -30},
                                      {-30, 0, 10, 15, 15, 10, 0, -30},
                                      {-40, -20, 0, 0, 0, 0, -20, -40},
                                      {-50, -40, -30, -30, -30, -30, -40, -50}};

// White Bishops
const int white_bishop_table[8][8] = {{-20, -10, -10, -10, -10, -10, -10, -20},
                                      {-10, 0, 0, 0, 0, 0, 0, -10},
                                      {-10, 0, 10, 10, 10, 10, 0, -10},
                                      {-10, 5, 5, 10, 10, 5, 5, -10},
                                      {-10, 0, 5, 10, 10, 5, 0, -10},
                                      {-10, 10, 10, 10, 10, 10, 10, -10},
                                      {-10, 5, 0, 0, 0, 0, 5, -10},
                                      {-20, -10, -10, -10, -10, -10, -10, -20}};

// Black Bishops
const int black_bishop_table[8][8] = {{-20, -10, -10, -10, -10, -10, -10, -20},
                                      {-10, 5, 0, 0, 0, 0, 5, -10},
                                      {-10, 10, 10, 10, 10, 10, 10, -10},
                                      {-10, 0, 5, 10, 10, 5, 0, -10},
                                      {-10, 5, 5, 10, 10, 5, 5, -10},
                                      {-10, 0, 10, 10, 10, 10, 0, -10},
                                      {-10, 0, 0, 0, 0, 0, 0, -10},
                                      {-20, -10, -10, -10, -10, -10, -10, -20}};

// White Rooks
const int white_rook_table[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},   {5, 10, 10, 10, 10, 10, 10, 5},
    {-5, 0, 0, 0, 0, 0, 0, -5}, {-5, 0, 0, 0, 0, 0, 0, -5},
    {-5, 0, 0, 0, 0, 0, 0, -5}, {-5, 0, 0, 0, 0, 0, 0, -5},
    {-5, 0, 0, 0, 0, 0, 0, -5}, {0, 0, 0, 5, 5, 0, 0, 0}};

// Black Rooks
const int black_rook_table[8][8] = {
    {0, 0, 0, 5, 5, 0, 0, 0},       {-5, 0, 0, 0, 0, 0, 0, -5},
    {-5, 0, 0, 0, 0, 0, 0, -5},     {-5, 0, 0, 0, 0, 0, 0, -5},
    {-5, 0, 0, 0, 0, 0, 0, -5},     {-5, 0, 0, 0, 0, 0, 0, -5},
    {5, 10, 10, 10, 10, 10, 10, 5}, {0, 0, 0, 0, 0, 0, 0, 0}};

// White Queens
const int white_queen_table[8][8] = {{-20, -10, -10, -5, -5, -10, -10, -20},
                                     {-10, 0, 0, 0, 0, 0, 0, -10},
                                     {-10, 0, 5, 5, 5, 5, 0, -10},
                                     {-5, 0, 5, 5, 5, 5, 0, -5},
                                     {0, 0, 5, 5, 5, 5, 0, -5},
                                     {-10, 5, 5, 5, 5, 5, 0, -10},
                                     {-10, 0, 5, 0, 0, 0, 0, -10},
                                     {-20, -10, -10, -5, -5, -10, -10, -20}};

// Black Queens
const int black_queen_table[8][8] = {{-20, -10, -10, -5, -5, -10, -10, -20},
                                     {-10, 0, 5, 0, 0, 0, 0, -10},
                                     {-10, 5, 5, 5, 5, 5, 0, -10},
                                     {0, 0, 5, 5, 5, 5, 0, -5},
                                     {-5, 0, 5, 5, 5, 5, 0, -5},
                                     {-10, 0, 5, 5, 5, 5, 0, -10},
                                     {-10, 0, 0, 0, 0, 0, 0, -10},
                                     {-20, -10, -10, -5, -5, -10, -10, -20}};

// TODO: King table

/**
 * @brief Computes a positional bonus of a given bitboard,
 * and adds the bonus to the raw_score value.
 *
 * @param score: A pointer to the score value in __eval
 * @param color: The color of the bitboard.
 * @param bb: A bitboard to compute the positional bonus
 * @param table: The table to use for computation.
 */
void __compute_position_bonus(int *score, enum PieceColor color, BITBOARD bb,
                              const int table[8][8]) {
  while (bb) {
    unsigned int lsb = POP_LSB(bb);
    unsigned int rank_idx = 7 - (lsb / 8);
    unsigned int file_idx = 7 - (lsb % 8);
    if (color == WHITE) {
      *score += table[rank_idx][file_idx];
    } else if (color == BLACK) {
      *score -= table[rank_idx][file_idx];
    }
  }
}

int __eval(ChessBitboards *bbs) {
  // White evals
  int w_pawn_score = __builtin_popcountll(bbs->white_pawns) * 100;
  int w_bishop_score = __builtin_popcountll(bbs->white_bishops) * 300;
  int w_knight_score = __builtin_popcountll(bbs->white_knights) * 300;
  int w_rook_score = __builtin_popcountll(bbs->white_rooks) * 500;
  int w_queen_score = __builtin_popcountll(bbs->white_queens) * 900;
  int w_king_score = __builtin_popcountll(bbs->white_king) * 9999900;

  // Black evals
  int b_pawn_score = __builtin_popcountll(bbs->black_pawns) * -100;
  int b_bishop_score = __builtin_popcountll(bbs->black_bishops) * -300;
  int b_knight_score = __builtin_popcountll(bbs->black_knights) * -300;
  int b_rook_score = __builtin_popcountll(bbs->black_rooks) * -500;
  int b_queen_score = __builtin_popcountll(bbs->black_queens) * -900;
  int b_king_score = __builtin_popcountll(bbs->black_king) * -9999900;

  int score = (w_pawn_score + w_bishop_score + w_knight_score + w_rook_score +
               w_queen_score + w_king_score + b_pawn_score + b_bishop_score +
               b_knight_score + b_rook_score + b_queen_score + b_king_score);

  //
  // Position Tables
  __compute_position_bonus(&score, WHITE, bbs->white_pawns, white_pawn_table);
  __compute_position_bonus(&score, BLACK, bbs->black_pawns, black_pawn_table);
  __compute_position_bonus(&score, WHITE, bbs->white_knights,
                           white_knight_table);
  __compute_position_bonus(&score, BLACK, bbs->black_knights,
                           black_knight_table);
  __compute_position_bonus(&score, WHITE, bbs->white_bishops,
                           white_bishop_table);
  __compute_position_bonus(&score, BLACK, bbs->black_bishops,
                           black_bishop_table);
  __compute_position_bonus(&score, WHITE, bbs->white_rooks, white_rook_table);
  __compute_position_bonus(&score, BLACK, bbs->black_rooks, black_rook_table);
  __compute_position_bonus(&score, WHITE, bbs->white_queens, white_queen_table);
  __compute_position_bonus(&score, BLACK, bbs->black_queens, black_queen_table);

  return score;
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

  bool found_legal_move = false;
  MoveArray potential_moves;

  engine_generate_pseudolegal_moves(bbs, magic, &potential_moves, turn);

  for (unsigned int i = 0; i < potential_moves.len; i++) {
    move_info_t move = potential_moves.moves[i];
    unsigned int from_pos = GET_FROM_POS(move);
    unsigned int to_pos = GET_TO_POS(move);
    Piece captured = engine_move(bbs, from_pos, to_pos);

    // Skip illegal moves (leaves own king in check)
    if (engine_color_in_check(bbs, magic, turn)) {
      if (move & FLAG_PROMOTION)
        engine_undo_promotion(bbs, to_pos, turn);
      engine_move(bbs, to_pos, from_pos);
      engine_undo_capture(bbs, &captured, to_pos);
      continue;
    }

    found_legal_move = true;
    int eval =
        __minimax(bbs, magic, depth - 1, turn == WHITE ? BLACK : WHITE, a, b);
    best_eval = (turn == WHITE) ? (eval > best_eval ? eval : best_eval)
                                : (eval < best_eval ? eval : best_eval);

    if (move & FLAG_PROMOTION)
      engine_undo_promotion(bbs, to_pos, turn);
    engine_move(bbs, to_pos, from_pos);
    engine_undo_capture(bbs, &captured, to_pos);

    if (turn == WHITE) {
      a = a >= best_eval ? a : best_eval;
    } else {
      b = b <= best_eval ? b : best_eval;
    }
    if (a >= b)
      break;
  }

  // No legal moves: checkmate or stalemate
  if (!found_legal_move) {
    if (engine_color_in_check(bbs, magic, turn)) {
      // Checkmate: worse the deeper it is (prefer faster mates)
      return turn == WHITE ? -9999900 - (int)depth : 9999900 + (int)depth;
    }
    return 0;
  }

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
  move_info_t best_move = 0;
  MoveArray potential_moves;
  int best_eval = turn == WHITE ? INT_MIN : INT_MAX;

  engine_generate_pseudolegal_moves(bbs, magic, &potential_moves, turn);

  for (unsigned int i = 0; i < potential_moves.len; i++) {
    move_info_t move = potential_moves.moves[i];
    unsigned int from_pos = GET_FROM_POS(move);
    unsigned int to_pos = GET_TO_POS(move);
    Piece captured =
        engine_move(bbs, from_pos, to_pos); // make the move (simulate it)

    // Skip illegal moves
    if (engine_color_in_check(bbs, magic, turn)) {
      if (move & FLAG_PROMOTION)
        engine_undo_promotion(bbs, to_pos, turn);
      engine_move(bbs, to_pos, from_pos);
      engine_undo_capture(bbs, &captured, to_pos);
      continue;
    }

    int eval = __minimax(bbs, magic, depth - 1, turn == WHITE ? BLACK : WHITE,
                         INT_MIN, INT_MAX);

    if ((turn == WHITE && eval > best_eval) ||
        (turn == BLACK && eval < best_eval)) {
      best_eval = eval;
      best_move = move;
    }

    // reset the move
    if (move & FLAG_PROMOTION)
      engine_undo_promotion(bbs, to_pos, turn);
    engine_move(bbs, to_pos, from_pos);
    engine_undo_capture(bbs, &captured, to_pos);
  }

  return (EvalResult){.best_move = best_move, .eval = best_eval};
}
