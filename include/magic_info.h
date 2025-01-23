#ifndef MAGIC_INFO_H
#define MAGIC_INFO_H

//
// Magic Information

// Rooks (straight)
typedef struct {
  unsigned long long ROOK_MAGICS[64];
  unsigned int ROOK_SHIFTS[64];
  unsigned long long BISHOP_MAGICS[64];
  unsigned int BISHOP_SHIFTS[64];
} MagicInfo;

/**
 * @brief Initialize the *_MAGICS and *_SHIFTS arrays.
 */
MagicInfo init_magic_info();

#endif // MAGIC_INFO_H
