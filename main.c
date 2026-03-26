#define CSV_IMPLEMENTATION
#include "lib/csv.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PATH_LEN 1024

int main(int argc, char **argv) {
  int opt;
  int option_index = 0;
  char file_path[PATH_LEN];

  static struct option long_options[] = {
      {"file", required_argument, 0, 'f'},
      {0, 0, 0, 0},
  };
  while ((opt = getopt_long(argc, argv, "f:", long_options, &option_index)) !=
         -1) {
    switch (opt) {
    case 'f':
      if (strlen(optarg) + 1 > PATH_LEN) {
        printf("maximum allowed file path length allowed is %d, file path "
               "length passed as argument is %lu\n",
               PATH_LEN, strlen(optarg) + 1);
        exit(EXIT_FAILURE);
      }
      snprintf(file_path, strlen(optarg) + 1, "%s", optarg);
      break;
    default:
      printf("Err\n");
      exit(EXIT_FAILURE);
    }
  }

  validate(file_path);

  return 0;
}
