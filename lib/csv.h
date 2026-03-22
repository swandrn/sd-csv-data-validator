#ifndef LIB_CSV_H
#define LIB_CSV_H
#endif // LIB_CSV_H

#define CSV_IMPLEMENTATION // Enables syntax highlighting - Developement only
#ifdef CSV_IMPLEMENTATION
#include <stdio.h>

static int validate(const char *path) {
  printf("hello from validate with path=%s\n", path);
  return 0;
}
#endif // CSV_IMPLEMENTATION
