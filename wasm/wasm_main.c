#include "bitboard.h"
#include "engine.h"
#include "uci.h"
#include "magic_info.h"
#include <emscripten.h>

#define DEFAULT_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define MAX_RESPONSE 1024
static char response[MAX_RESPONSE];

static ChessBitboards bbs;
static MagicInfo magic;

static BITBOARD *rook_move_table[64];
static BITBOARD *bishop_move_table[64];

EMSCRIPTEN_KEEPALIVE
void wasm_init() {
  bb_init_chess_boards(&bbs, DEFAULT_FEN);
  magic = init_magic_info();
  engine_setup(&bbs, rook_move_table, bishop_move_table, &magic);
  bbs.rook_move_table = rook_move_table;
  bbs.bishop_move_table = bishop_move_table;
}

EMSCRIPTEN_KEEPALIVE
const char *wasm_process_uci_command(const char *cmd) {
  String cmd_str = str_create(cmd);
  process_uci_command(&cmd_str, &bbs, &magic, response, MAX_RESPONSE);
  return response;
}
