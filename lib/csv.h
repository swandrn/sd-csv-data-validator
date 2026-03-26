#ifndef LIB_CSV_H
#define LIB_CSV_H

#ifdef __cplusplus
extern "C" {
#endif
#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CSV_CAPACITY 4096 // Number of rows allowed in a csv
#define ROW_CAPACITY 256  // Number of fields allowed in a row

typedef enum {
  UNDEFINED_TYPE = 0,
  NULL_TYPE = 1,
  INT_TYPE = 2,
  FLOAT_TYPE = 3,
  STRING_TYPE = 4,
} FieldType;

typedef struct {
  FieldType field_type;
} Field;

typedef struct {
  Field *fields;
  size_t size;
  size_t capacity;
} Row;

typedef struct {
  Row *rows;
  size_t size;
  size_t capacity;
} CSV;

typedef struct {
  size_t size;
  size_t capacity;
  char *data;
} Region;

#ifdef __cplusplus
extern "C" Region region_malloc(size_t capacity);
extern "C" void *region_alloc(Region *r, size_t size);
extern "C" void region_reset(Region *r);
extern "C" void region_free(Region *r);
extern "C" void read_csv(Region *csv_r, CSV *csv, const char *path);
extern "C" int validate(const char *path);
#else
extern Region region_malloc(size_t capacity);
extern void *region_alloc(Region *r, size_t size);
extern void region_reset(Region *r);
extern void region_free(Region *r);
extern void read_csv(Region *csv_r, CSV *csv, const char *path);
extern int validate(const char *path);
#endif

#ifdef __cplusplus
}
#endif

#endif // LIB_CSV_H

#ifdef CSV_IMPLEMENTATION
#include <ctype.h>
#include <errno.h>

Region region_malloc(size_t capacity) {
  void *data = malloc(capacity);
  assert(data != NULL);
  Region r = {
      .size = 0,
      .capacity = capacity,
      .data = data,
  };
  return r;
}

// Allocate within the region
void *region_alloc(Region *r, size_t size) {
  // bitwise operation to push r->size to the next block if needed
  size_t a = alignof(max_align_t);
  size_t aligned_offset = (r->size + (a - 1)) & ~(a - 1);

  size_t padding = aligned_offset - r->size;

  assert(r->size + padding + size <= r->capacity);

  void *result = &r->data[aligned_offset];
  r->size += size + padding;
  return result;
}
void region_reset(Region *r) { r->size = 0; }

void region_free(Region *r) {
  if (!r)
    return;
  free(r->data);
  r->data = NULL;
  r->capacity = r->size = 0;
}

// Checks that a file is a CSV, exits with non zero otherwise
void file_is_csv(const char *path) {
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

void set_field_type(Field *csvf, const char *field) {
  csvf->field_type = UNDEFINED_TYPE;
  int field_len = strlen(field);
  if (field_len == 0) {
    csvf->field_type = NULL_TYPE;
    return;
  }
  bool is_number = true;
  for (int i = 0; i < field_len; i++) {
    if (!isdigit(field[i])) {
      is_number = false;
      break;
    };
  }
  if (is_number) {
    csvf->field_type = INT_TYPE;
    return;
  }
  csvf->field_type = STRING_TYPE;
  return;
}

void read_csv(Region *csv_r, CSV *csv, const char *path) {
  char line[4096];
  FILE *f = fopen(path, "r");
  if (f == NULL) {
    fprintf(stderr, "error opening %s: %s\n", path, strerror(errno));
    region_free(csv_r);
    return;
  }

  while (fgets(line, sizeof(line), f)) {
    line[strcspn(line, "\n")] = '\0';

    char *p = line;
    char *field;

    int row_idx = 0;
    while ((field = strsep(&p, ",")) != NULL) {
      size_t field_idx = csv->rows[row_idx].size++;
      if (field_idx >= csv->rows[row_idx].capacity) {
        fprintf(stderr, "maximum capacity of %zu has been reached\n",
                csv->rows[row_idx].capacity);
        break;
      }
      set_field_type(&csv->rows[row_idx].fields[csv->rows[row_idx].size++],
                     field);
    }
    row_idx++;
  }
  fclose(f);
}

int validate(const char *path) {
  Region csv_region = region_malloc(1024 * 1024 * 50);
  CSV *csv = (CSV *)region_alloc(&csv_region, sizeof(*csv));
  csv->rows =
      (Row *)region_alloc(&csv_region, CSV_CAPACITY * sizeof(*csv->rows));
  csv->rows->fields = (Field *)region_alloc(
      &csv_region, CSV_CAPACITY * ROW_CAPACITY * sizeof(*csv->rows->fields));
  csv->rows->capacity = ROW_CAPACITY;
  csv->rows->size = 0;
  read_csv(&csv_region, csv, path);
  region_free(&csv_region);
  return 0;
}
#endif // CSV_IMPLEMENTATION
