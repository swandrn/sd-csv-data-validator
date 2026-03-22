#ifndef LIB_CSV_H
#define LIB_CSV_H
#endif // LIB_CSV_H

#define CSV_IMPLEMENTATION // Enables syntax highlighting - Developement only
#ifdef CSV_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Checks that a file is a CSV, exits with non zero otherwise
static void file_is_csv(const char *path) {
  // ends with .csv
  const char *ext = ".csv";
  size_t path_len = strlen(path);
  size_t ext_len = strlen(ext);
  if (ext_len > path_len) {
    fprintf(stderr,
            "length of file extension (%zu) is longer than file path (%zu)\n",
            ext_len, path_len);
    exit(EXIT_FAILURE);
  }
  if (strncmp(path + path_len - ext_len, ext, ext_len) != 0) {
    fprintf(stderr, "file extension is not .csv\n");
    exit(EXIT_FAILURE);
  }
}

static int validate(const char *path) {
  printf("hello from validate with path=%s\n", path);
  return 0;
}
#endif // CSV_IMPLEMENTATION
