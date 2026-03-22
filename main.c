#include "lib/csv.h"
#include <stdio.h>

#define CSV_IMPLEMENTATION

int main(int argc, char **argv) {
  for (int i = 0; i < argc; i++) {
    printf("argv=%s\n", argv[i]);
  }
  validate("tmp");
  return 0;
}
