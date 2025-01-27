#include "magic_info.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void __set_magic_info(unsigned long long *magics, unsigned int *shifts,
                      const char *filename) {
  FILE *fp;

  fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "*-magic.out file not found. Create it with 'make "
                    "*_magic'. (Replace the '*' with 'rook' or 'bishop').\n");
    exit(1);
  }

  size_t BUFFER_SIZE = 1024;
  char *buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));

  // Magics
  ssize_t read = getline(&buffer, &BUFFER_SIZE, fp);
  buffer[read] = 0;
  String line = str_create(buffer);
  Vec magic_numbers = str_split(&line);
  assert(magic_numbers.len == 64);
  for (unsigned int i = 0; i < magic_numbers.len; i++) {
    magics[i] = strtoull(vec_get(&magic_numbers, i), NULL, 10);
  }
  str_free(&line);
  vec_free(&magic_numbers);

  // Shifts
  memset(buffer, 0, BUFFER_SIZE * sizeof(char));
  read = getline(&buffer, &BUFFER_SIZE, fp);
  buffer[read] = 0;
  line = str_create(buffer);
  Vec shifts_vec = str_split(&line);
  assert(shifts_vec.len == 64);
  for (unsigned int i = 0; i < shifts_vec.len; i++) {
    shifts[i] = strtoull(vec_get(&shifts_vec, i), NULL, 10);
  }
  str_free(&line);
  vec_free(&shifts_vec);

  free(buffer);
  fclose(fp);
}

/**
 * @brief Initialize the *_MAGICS and *_SHIFTS arrays.
 */
MagicInfo init_magic_info() {
  // NOTE: Since this is the only function, I will keep this in the header (for
  // now).

  MagicInfo magic_info;

  //
  // Rook info
  __set_magic_info(magic_info.ROOK_MAGICS, magic_info.ROOK_SHIFTS,
                   "rook-magics.out");

  //
  // Bishop info
  __set_magic_info(magic_info.BISHOP_MAGICS, magic_info.BISHOP_SHIFTS,
                   "bishop-magics.out");

  return magic_info;
}
