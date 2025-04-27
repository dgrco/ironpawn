#include "bitboard.h"
#include "uci.h"
#include "magic_info.h"
#include <emscripten.h>

#define DEFAULT_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define MAX_RESPONSE 1024
static char response[MAX_RESPONSE];

static bool bbs_init = false;
static ChessBitboards bbs;
static bool magic_init = false;
static MagicInfo magic;

EMSCRIPTEN_KEEPALIVE
void wasm_init_bitboards() {
  bbs = bb_init_chess_boards(DEFAULT_FEN);
  bbs_init = true;
}

EMSCRIPTEN_KEEPALIVE
void wasm_init_magic() {
  magic = init_magic_info();
  magic_init = true;
}

EMSCRIPTEN_KEEPALIVE
const char *wasm_process_uci_command(const char *cmd) {
  if (!bbs_init) {
    snprintf(response, MAX_RESPONSE, "Error: Bitboards not initialized. Make sure 'wasm_init_bitboards()' is called prior to processing a UCI command.\n");
    return response;
  }
  if (!magic_init) {
    snprintf(response, MAX_RESPONSE, "Error: MagicInfo not initialized. Make sure 'wasm_init_magic()' is called prior to processing a UCI command.\n");
    return response;
  }
  String cmd_str = str_create(cmd);
  process_uci_command(&cmd_str, &bbs, &magic, response, MAX_RESPONSE);
  return response;
}
