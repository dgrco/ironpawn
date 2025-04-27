#ifndef UCI_H
#define UCI_H

#include "bitboard.h"
#include "magic_info.h"
#include "utils.h"

void handle_uci_init(char *response, const int MAX_RESPONSE);
void handle_position(Vec *tokens, ChessBitboards *bbs, char *response,
                     const int MAX_RESPONSE);
void handle_go(Vec *tokens, ChessBitboards *bbs, MagicInfo *magic,
               char *response, const int MAX_RESPONSE);

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
                        char *response, const int MAX_RESPONSE);

#endif // UCI_H
