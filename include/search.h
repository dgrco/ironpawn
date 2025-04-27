#ifndef SEARCH_H
#define SEARCH_H

#include "bitboard.h"
#include "engine.h"

typedef struct {
  move_info_t best_move;
  int eval;
} EvalResult;

/**
 * @brief Perform a Minimax search of a certain depth.
 *
 * @param bbs: An existing ChessBitboards object.
 * @param magic: An existing MagicInfo object.
 * @param depth: The number of half-moves to search.
 * @param turn: the color whose turn it is to move.
 * @return An EvalResult with the best move and its evaluation value.
 */
EvalResult search(ChessBitboards *bbs, MagicInfo *magic, unsigned int depth,
                  enum PieceColor turn);

#endif // SEARCH_H
