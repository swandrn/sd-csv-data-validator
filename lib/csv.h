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
extern "C" int csv_parse(int max_field_size, FILE *fp, CSV *csv);
extern "C" int read_csv(Region *csv_r, CSV *csv, const char *path);
extern "C" int validate(const char *path);
#else
extern Region region_malloc(size_t capacity);
extern void *region_alloc(Region *r, size_t size);
extern void region_reset(Region *r);
extern void region_free(Region *r);
extern int csv_parse(int max_field_size, FILE *fp, CSV *csv);
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

bool csv_is_line_ending(char c, FILE *fp) {
  if (c == '\n') {
    return true;
  }

  if (c == '\r') {
    int next = fgetc(fp);
    if (next != '\n' && next != EOF) {
      ungetc(next, fp);
    }
    return true;
  }
  return false;
}

bool str_is_int(const char *s) {
  while (isspace((unsigned char)*s))
    s++;
  if (*s == '\0')
    return false;

  char *end;
  errno = 0;
  strtol(s, &end, 10);

  if (s == end)
    return false; // no conversion happened

  while (isspace((unsigned char)*end))
    end++;
  if (*end != '\0')
    return false; // extra junk at end

  return true;
}

bool str_is_float(const char *s) {
  while (isspace((unsigned char)*s))
    s++;
  if (*s == '\0')
    return false;

  char *end;
  errno = 0;
  strtof(s, &end);

  if (s == end)
    return false; // no conversion happened

  while (isspace((unsigned char)*end))
    end++;
  if (*end != '\0')
    return false; // extra junk at end

  return true;
}

FieldType set_field_type(Field *csvf, const char *field) {
  csvf->field_type = UNDEFINED_FIELD;
  if (field == NULL)
    return csvf->field_type;
  int field_len = strlen(field);
  if (field_len == 0) {
    return csvf->field_type = NULL_FIELD;
  }
  if (str_is_int(field)) {
    return csvf->field_type = INT_FIELD;
  }
  if (str_is_float(field))
    return csvf->field_type = FLOAT_FIELD;
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

// Terminates the current field buffer, increments the number of fields of the
// current row, and sets the type of the field
void commit_field(char *field_buf, int buf_idx, CSV *csv) {
  field_buf[buf_idx] = '\0';
  size_t field_idx = csv->rows[csv->size].size++;
  set_field_type(&csv->rows[csv->size].fields[field_idx], field_buf);
}

// Parse CSV and returns the number of rows or -1 if there is an error
int csv_parse(int max_field_size, FILE *fp, CSV *csv) {
  int c;
  int i = 0;
  char field_buf[max_field_size];
  ParserState state = START_FIELD;

  if (max_field_size <= 0 || fp == NULL || csv == NULL)
    return -1;

  while (i < max_field_size - 1 && c != EOF) {
    c = fgetc(fp);

    switch (state) {
    case START_FIELD:
      if (c == EOF) {
        return csv->size;
      } else if (csv_is_line_ending(c, fp)) {
        commit_field(field_buf, i, csv);
        csv->size++;
        i = 0;
        state = START_FIELD;
        break;
      } else if (c == '"') {
        state = IN_QUOTED;
        break;
      } else if (c == ',') {
        commit_field(field_buf, i, csv);
        i = 0;
        state = START_FIELD;
        break;
      } else {
        state = IN_UNQUOTED;
        field_buf[i++] = (char)c;
        break;
      }
      break;
    case IN_UNQUOTED:
      if (c == '"') {
        CSV_FPRINTF(stderr, "unexpected '\"' on row %d field %d\n",
                    csv->size + 1, csv->rows[csv->size].size + 1);
        return -1;
      } else if (c == ',') {
        commit_field(field_buf, i, csv);
        i = 0;
        state = START_FIELD;
        break;
      } else if (csv_is_line_ending(c, fp)) {
        commit_field(field_buf, i, csv);
        csv->size++;
        i = 0;
        state = START_FIELD;
        break;
      }
      field_buf[i++] = (char)c;
      break;
    case IN_QUOTED:
      if (c == '"') {
        state = AFTER_QUOTE;
        continue;
      }
      field_buf[i++] = (char)c;
      break;
    case AFTER_QUOTE:
      if (c == '"') {
        int next = fgetc(fp);
        if (next == ',' || next == '\0') {
          commit_field(field_buf, i, csv);
          i = 0;
          state = START_FIELD;
          break;
        }
        ungetc(next, fp);
        field_buf[i++] = (char)c;
        break;
      } else if (c == ',') {
        commit_field(field_buf, i, csv);
        i = 0;
        state = START_FIELD;
        break;
      } else if (csv_is_line_ending(c, fp)) {
        commit_field(field_buf, i, csv);
        csv->size++;
        i = 0;
        state = START_FIELD;
        break;
      } else {
        state = IN_QUOTED;
        field_buf[i++] = (char)c;
        break;
      }
    }
  }
  if (c != EOF) {
    CSV_FPRINTF(stderr, "max field size reached on row %d\n", csv->size);
    return -1;
  }
  return csv->size;
}

int read_csv(Region *csv_r, CSV *csv, const char *path) {
  if (csv_r == NULL || csv == NULL || path == NULL) {
    CSV_FPRINTF(stderr, "one or more pointer value is NULL\n");
    return 0;
  }
  FILE *f = fopen(path, "r");
  if (f == NULL) {
    CSV_FPRINTF(stderr, "error opening %s: %s\n", path, strerror(errno));
    region_free(csv_r);
    return 0;
  }

  csv_parse(4096, f, csv);
  fclose(f);
  return csv->size;
}

int validate(const char *path) {
  Region csv_region = region_malloc(1024 * 1024 * 50);
  CSV *csv = (CSV *)region_alloc(&csv_region, sizeof(*csv));
  csv->capacity = CSV_CAPACITY;
  csv->size = 0;
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
