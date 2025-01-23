#ifndef TYPES_H
#define TYPES_H

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

/////////////////////////////////
/// Struct/Enum Declarations
typedef struct {
  char *data;
  size_t len;
} String;

typedef struct {
  void **data;
  size_t len;
  size_t cap;
} Vec;

typedef struct {
  String *board_str;
  Vec *moves;  
} Board;

/// NOTE: only to be used for type-specific parsing purposes
enum VecType { INT, FLOAT, STRING };
/////////////////////////////////

/// Function Declarations

/////////////////////////////////
/// String

/// Create a "dead" string (a string with empty defaults).
String str_dead();

/// Create a new string, initialized with a given string literal
String str_create(const char *initial);

/// Create a new string from a vector of strings, each element is separated by a space.
String str_create_from_strvec(Vec *vec);

/// Free the string and reset its struct components
void str_free(String *str);

/// Read a string from stdin to an existing String object
void str_read_from_stdin(String *str, unsigned long maxlen);

/// Check if a String equals a string literal
bool str_eq(char *str, const char *other); 

/// Strip leading and trailing whitespace from a String
void str_strip(String *str);

/// Split a string into tokens (whitespace-separated words)
Vec str_split(String *str);

/// Replace a String's contents with another string.
/// This is useful if a string is intended to be mutable.
void str_replace(String *str, char *new_str, size_t maxlen);

/// Print a String
void str_println(String *str);

/// Print the debug info for a String
void str_printdebug(String *str);

/////////////////////////////////

/////////////////////////////////
/// Vector

/// Create a "dead" vector (one with empty defaults).
Vec vec_dead();

/// Create a new empty vector.
Vec vec_create();

/// Free a vector and its inner contents. NOTE: the contents must be heap
/// allocated.
void vec_free(Vec *vec);

/// Free a reference vector.
void vec_freeref(Vec *vec);

/// Append to a vector. NOTE: the item to be appended must be heap allocated.
void vec_append(Vec *vec, void *item);

/// Get element from the vector at a given index. NOTE: this returns the pointer
/// due to generics.
void *vec_get(Vec *vec, size_t index);

/// Check if the vector contains an element.
bool vec_contains(Vec *vec, enum VecType type, void *element);

/// Get the first index of an element, returns -1 if no element is found.
int vec_indexof(Vec *vec, enum VecType type, void *element);

/// Return a (reference) sub-vector of a given vector.
/// If endpos is the end of the vector, you can set endpos to -1.
/// NOTE: This operation does not create a copy of the elements, it merely references them.
/// So avoid using vec_free() on the returned vector, as it will cause a double free.
/// Instead, use vec_freeref().
Vec vec_refsubvec(Vec *vec, size_t startpos, ssize_t endpos);

/// Print the contents of the Vec
void vec_println(Vec *vec, enum VecType type);

/////////////////////////////////

#endif // TYPES_H
