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

#ifdef CSV_STDIO
#define CSV_FPRINTF fprintf
#else
#define CSV_FPRINTF(...) ((void)0)
#endif

#define CSV_CAPACITY 4096 // Number of rows allowed in a csv
#define ROW_CAPACITY 256  // Number of fields allowed in a row

typedef enum {
  UNDEFINED_FIELD = 0,
  NULL_FIELD = 1,
  INT_FIELD = 2,
  FLOAT_FIELD = 3,
  STRING_FIELD = 4,
  BOOLEAN_FIELD = 5,
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

typedef enum {
  START_FIELD,
  IN_UNQUOTED,
  IN_QUOTED,
  AFTER_QUOTE,
} ParserState;

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
extern "C" char *csv_getfield(char **buf);
extern "C" int read_csv(Region *csv_r, CSV *csv, const char *path);
extern "C" int validate(const char *path);
#else
extern Region region_malloc(size_t capacity);
extern void *region_alloc(Region *r, size_t size);
extern void region_reset(Region *r);
extern void region_free(Region *r);
extern char *csv_getfield(char **buf);
extern int read_csv(Region *csv_r, CSV *csv, const char *path);
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
    CSV_FPRINTF(
        stderr,
        "length of file extension (%zu) is longer than file path (%zu)\n",
        ext_len, path_len);
    exit(EXIT_FAILURE);
  }
  if (strncmp(path + path_len - ext_len, ext, ext_len) != 0) {
    CSV_FPRINTF(stderr, "file extension is not .csv\n");
    exit(EXIT_FAILURE);
  }
}

FieldType set_field_type(Field *csvf, const char *field) {
  csvf->field_type = UNDEFINED_FIELD;
  int field_len = strlen(field);
  if (field_len == 0) {
    return csvf->field_type = NULL_FIELD;
  }
  bool is_number = true;
  for (int i = 0; i < field_len; i++) {
    if (!isdigit(field[i])) {
      is_number = false;
      break;
    };
  }
  if (is_number) {
    return csvf->field_type = INT_FIELD;
  }
  size_t bufsize = strlen(field) + 1;
  char lower_field[bufsize];
  for (size_t i = 0; i < bufsize; i++) {
    lower_field[i] = tolower(field[i]);
  }
  if (strcmp(lower_field, "true") == 0 || strcmp(lower_field, "false") == 0)
    return csvf->field_type = BOOLEAN_FIELD;
  // TODO: Add check of properly escaped characters
  return csvf->field_type = STRING_FIELD;
}

char *csv_getline(char *buf, int size, FILE *fp) {
  int c;
  int i = 0;

  if (size <= 0 || buf == NULL || fp == NULL)
    return NULL;

  while (i < size - 1) {
    c = fgetc(fp);

    if (c == EOF) {
      break;
    }

    if (c == '\n') {
      break;
    }

    if (c == '\r') {
      int next = fgetc(fp);
      if (next != '\n' && next != EOF) {
        ungetc(next, fp);
      }
      break;
    }

    buf[i++] = (char)c;
  }

  if (i == 0 && c == EOF) {
    return NULL;
  }

  buf[i] = '\0';
  return buf;
}

// Separate fields on commas and move pointer the start of the next field
char *csv_getfield(char **buf) {
  ParserState state = START_FIELD;
  char *start = *buf;
  char *out = start;

  if (start == NULL) {
    return NULL;
  }

  while (*out != '\0') {
    switch (state) {
    case START_FIELD:
      if (**buf == '"') {
        state = IN_QUOTED;
        (*buf)++;
      } else if (**buf == ',') {
        *out = '\0';
        (*buf)++;
        return start;
      } else {
        state = IN_UNQUOTED;
        // Append char and slide pointer forward by one
        *out = **buf;
        out++;
        (*buf)++;
      }
      break;
    case IN_UNQUOTED:
      if (**buf == '"') {
        buf = NULL;
        return NULL;
      } else if (**buf == ',') {
        *out = '\0';
        (*buf)++;
        return start;
      }
      // Append char and slide pointer forward by one
      *out = **buf;
      out++;
      (*buf)++;
      break;
    case IN_QUOTED:
      if (**buf == '"') {
        state = AFTER_QUOTE;
        (*buf)++;
        continue;
      }
      // Append char and slide pointer forward by one
      *out = **buf;
      out++;
      (*buf)++;
      break;
    case AFTER_QUOTE:
      if (**buf == '"') {
        char *next = *buf + 1;
        if (*next == ',' || *next == '\0') {
          *out = '\0';
          out++;
          (*buf)++;
          break;
        }
        // Append char and slide pointer forward by one
        *out = **buf;
        out++;
        (*buf)++;
      } else if (**buf == ',') {
        *out = '\0';
        (*buf)++;
        return start;
      } else {
        state = IN_QUOTED;
        // Append char and slide pointer forward by one
        *out = **buf;
        out++;
        (*buf)++;
      }
      break;
    }
  }
  *buf = NULL;
  return start;
}

int read_csv(Region *csv_r, CSV *csv, const char *path) {
  if (csv_r == NULL || csv == NULL || path == NULL) {
    CSV_FPRINTF(stderr, "one or more pointer value is NULL\n");
    return 0;
  }
  char line[4096];
  FILE *f = fopen(path, "r");
  if (f == NULL) {
    CSV_FPRINTF(stderr, "error opening %s: %s\n", path, strerror(errno));
    region_free(csv_r);
    return 0;
  }

  int row_idx = 0;
  while (csv_getline(line, sizeof(line), f)) {
    line[strcspn(line, "\n")] = '\0';

    char *p = line;
    char *field;

    while ((field = csv_getfield(&p)) != NULL) {
      size_t field_idx = csv->rows[row_idx].size++;
      if (field_idx >= csv->rows[row_idx].capacity) {
        CSV_FPRINTF(stderr, "maximum capacity of %zu has been reached\n",
                    csv->rows[row_idx].capacity);
        break;
      }
      set_field_type(&csv->rows[row_idx].fields[field_idx], field);
    }
    row_idx++;
  }
  fclose(f);
  return row_idx;
}

int validate(const char *path) {
  Region csv_region = region_malloc(1024 * 1024 * 50);
  CSV *csv = (CSV *)region_alloc(&csv_region, sizeof(*csv));
  csv->capacity = CSV_CAPACITY;
  csv->rows =
      (Row *)region_alloc(&csv_region, csv->capacity * sizeof(*csv->rows));
  for (size_t i = 0; i < csv->capacity; i++) {
    csv->rows[i].capacity = ROW_CAPACITY;
    csv->rows[i].fields = (Field *)region_alloc(
        &csv_region, csv->rows[i].capacity * sizeof(*csv->rows->fields));
    csv->rows[i].size = 0;
  }
  read_csv(&csv_region, csv, path);
  region_free(&csv_region);
  return 0;
}
#endif // CSV_IMPLEMENTATION
