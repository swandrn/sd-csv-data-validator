#ifndef LIB_CSV_H
#define LIB_CSV_H
#endif // LIB_CSV_H

#define CSV_IMPLEMENTATION // Enables syntax highlighting - Developement only
#ifdef CSV_IMPLEMENTATION
#include <stdio.h>

static int validate(const char *path) {
  printf("hello from validate\n");
  return 0;
}
#endif // CSV_IMPLEMENTATION
