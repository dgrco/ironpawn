#include "utils.h"
#include "engine.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>


/////////////////////////////////
/// String

/// Create a "dead" string (a string with empty defaults)
String str_dead() {
  return (String){.data = NULL, .len = 0};
}

/// Create a new string, initialized with a given string literal
String str_create(const char *initial) {
  String str;
  str.len = strlen(initial);
  str.data = (char *)calloc(str.len + 1, sizeof(char));
  if (str.data) {
    strncpy(str.data, initial, str.len);
  }
  return str;
}

/// Create a new string from a vector of strings, each element is separated by a space.
String str_create_from_strvec(Vec *vec) {
  size_t str_len = 0;
  for (size_t i = 0; i < vec->len; i++) {
    str_len += strlen(vec->data[i]);
  }
  str_len += vec->len - 1; // for spaces between elements
  str_len += 1; // for null terminator

  char *str_res = (char *)calloc(str_len, sizeof(char));

  for (size_t i = 0; i < vec->len; i++) {
    strcat(str_res, vec_get(vec, i));
    if (i < vec->len - 1) {
      strcat(str_res, " ");
    }
  }
  String str = {.data = str_res, .len = str_len};
  return str;
}

/// Free the string and reset its struct components
void str_free(String *str) {
  if (str && str->data) {
    free(str->data);
    str->data = NULL;
    str->len = 0;
  }
}

/// Read a string from stdin to an existing String object
void str_read_from_stdin(String *str, unsigned long maxlen) {
  char buffer[maxlen + 1];
  memset(buffer, 0, (maxlen + 1) * sizeof(char));
  fgets(buffer, maxlen, stdin);
  str->len = strnlen(buffer, maxlen);
  str->data = (char*)realloc(str->data, str->len + 1);
  if (!str->data) {
    fprintf(stderr, "Cannot read from standard input. Reallocation issue.\n");
    exit(1);
  }
  strncpy(str->data, buffer, str->len);
}

/// Check if a string equals another string literal
bool str_eq(char *str, const char *other) {
  return !strcmp(str, other);
}

/// Strip leading and trailing whitespace from a String
/// NOTE: treats newline as whitespace then appends it again after truncating.
void str_strip(String *str) {
  // Find first non-whitespace character
  char *start = str->data;
  while (isspace((unsigned char)*start)) {
    start++;
  }
  // Find last non-whitespace character
  char *end = str->data + str->len - 1;
  while (end > start && (isspace((unsigned char)*end) || (unsigned char)*end == '\n')) {
    end--;
  }

  // Set newline and null terminator
  *(end + 1) = '\n';
  *(end + 2) = '\0';

  if (start != str->data) {
    memmove(str->data, start, end - start + 3); // + 3 since we need to account for newline and null terminator
  }
  str->len = end - start + 2; // + 2 since we need to account for newline and null terminator
}

/// Split a string into tokens (whitespace-separated words).
Vec str_split(String *str) {
  Vec words = vec_create();
  unsigned int buffer_len = 0;
  char *buffer = (char *)malloc(str->len * sizeof(char));

  char *curr = str->data;
  while (*curr != '\0') {
    if (isspace((unsigned char)*curr) || (unsigned char)*curr == '\n') {
      if (buffer_len > 0) {
        buffer = (char *)realloc(buffer, buffer_len + 1);
        buffer[buffer_len] = '\0';
        vec_append(&words, buffer);
        buffer = (char *)malloc(str->len * sizeof(char));
        buffer_len = 0;
      }
    } else {
      buffer[buffer_len++] = *curr;
    }
    curr += 1;
  }

  if (buffer_len > 0) {
    buffer = (char *)realloc(buffer, buffer_len + 1);
    buffer[buffer_len] = '\0';
    vec_append(&words, buffer);
  } else {
    free(buffer);
  }

  return words;
}

/// Replace a String's contents with another string.
/// This is useful if a string is intended to be mutable.
void str_replace(String *str, char *new_str, size_t maxlen) {
  assert(str && str->data && new_str);
  size_t new_str_len = strnlen(new_str, maxlen);
  str->data = (char *)realloc(str->data, new_str_len + 1);
  if (!str->data) {
    fprintf(stderr, "Could not replace string. Memory reallocation issue.\n");
    exit(1);
  }
  memset(str->data, 0, maxlen); // ensure null termination
  strncpy(str->data, new_str, new_str_len);
}

/// Print a String
void str_println(String *str) {
  printf("%s\n", str->data);
}

/// Print the debug info for a String
void str_printdebug(String *str) {
  printf("{content: \"%s\", length: %lu, String address: %p, data address: %p}\n", str->data, str->len, str, str->data);
}

/////////////////////////////////

/////////////////////////////////
/// Vector

/// Create a "dead" vector (one with empty defaults).
Vec vec_dead() {
  return (Vec){.data = NULL, .len = 0, .cap = 0};
}

/// Create a new empty vector.
Vec vec_create() {
  Vec vec = {.data = NULL, .len = 0, .cap = 8};
  vec.data = (void**)calloc(vec.cap, sizeof(void*));
  return vec;
}

/// Free a vector and its inner contents. NOTE: the contents must be heap allocated.
void vec_free(Vec *vec) {
  if (!vec) {
    return;
  }

  for (size_t i = 0; i < vec->len; i++) {
    if (vec->data[i]) {
      free(vec->data[i]);
      vec->data[i] = NULL;
    }
  }
  if (vec->data) {
    free(vec->data);
    vec->data = NULL;
  }

  vec->len = 0;
  vec->cap = 0;
}

/// Free a reference vector.
void vec_freeref(Vec *vec) {
  if (vec && vec->data) {
    free(vec->data);
    vec->data = NULL;
  }
}

/// Helper: grow the vector's capacity by a factor of 2
void vec_grow(Vec *vec) {
  void **new_data = (void**)realloc(vec->data, 2 * vec->cap * sizeof(void*));
  if (new_data) {
    vec->data = new_data;
    vec->cap *= 2;
  }
}

/// Append to a vector. NOTE: the item to be appended must be heap allocated.
void vec_append(Vec *vec, void *item) {
  if (vec->len == vec->cap) {
    vec_grow(vec);
  }
  vec->data[vec->len] = item;
  vec->len += 1;
}

/// Get element from the vector at a given index. NOTE: this returns the pointer due to generics.
void *vec_get(Vec *vec, size_t index) {
  assert(index < vec->len);
  return vec->data[index];
}

/// Check if the vector contains an element.
bool vec_contains(Vec *vec, enum VecType type, void *element) {
  if (type == STRING) {
    for (size_t i = 0; i < vec->len; i++) {
      if (str_eq(vec->data[i], (char*)element)) {
        return true;
      }
    }
  } else if (type == INT) {
    for (size_t i = 0; i < vec->len; i++) {
      if (*(int*)vec->data[i] == *(int*)element) {
        return true;
      }
    }
  } else if (type == FLOAT) {
    for (size_t i = 0; i < vec->len; i++) {
      if (*(float*)vec->data[i] == *(float*)element) {
        return true;
      }
    }
  } else {
    fprintf(stderr, "Unsupported type: %d\n", type);
    exit(1);
  }

  return false;
}

/// Get the first index of an element, returns -1 if no element is found.
int vec_indexof(Vec *vec, enum VecType type, void *element) {
  if (type == STRING) {
    for (size_t i = 0; i < vec->len; i++) {
      if (str_eq(vec->data[i], (char *)element)) {
        return i;
      }
    }
  } else if (type == INT) {
    for (size_t i = 0; i < vec->len; i++) {
      if (*(int *)vec->data[i] == *(int *)element) {
        return i;
      }
    }
  } else if (type == FLOAT) {
    for (size_t i = 0; i < vec->len; i++) {
      if (*(float *)vec->data[i] == *(float *)element) {
        return i;
      }
    }
  } else {
    fprintf(stderr, "Unsupported type: %d\n", type);
    exit(1);
  }
  return -1;
}

/// Return a (reference) sub-vector of a given vector.
/// If endpos is the end of the vector, you can set endpos to -1.
/// NOTE: This operation does not create a copy of the elements, it merely references them.
/// So avoid using vec_free() on the returned vector, as it will cause a double free.
/// Instead, use vec_freeref().
Vec vec_refsubvec(Vec *vec, size_t startpos, ssize_t endpos) {
  assert(startpos < vec->len && (endpos == -1 || (size_t) endpos < vec->len));
  if (endpos == -1) {
    endpos = vec->len;
  }
  Vec subvec = vec_create();
  for (size_t i = startpos; i < (size_t)endpos; i++) {
    vec_append(&subvec, vec->data[i]);
  }
  return subvec;
}

/// Print the contents of the Vec. NOTE: if the type is incorrect there will be undefined behavior.
void vec_print(Vec *vec, enum VecType type) {
  char fmt[10] = {0};
  switch (type) {
    case INT: {
      strncpy(fmt, "%ld", 4);
      break;
    };
    case FLOAT: {
      strncpy(fmt, "%f", 3);
      break;
    };
    case STRING: {
      strncpy(fmt, "%s", 3);
      break;
    };
  }

  printf("[");
  for (size_t i = 0; i < vec->len; i++) {
    if (type == INT) {
      printf(fmt, *(long*)vec->data[i]);
    } else if (type == FLOAT) {
      printf(fmt, *(float*)vec->data[i]);
    } else if (type == STRING) {
      printf(fmt, (char *)vec->data[i]);
    }

    if (i < vec->len - 1) {
      printf(", ");
    }
  }
  printf("]\n");
}

/////////////////////////////////

