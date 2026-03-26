#include <gtest/gtest.h>
extern "C" {
#include "csv.h"
}

#define TEST_CSV_CAPACITY 64
#define TEST_ROW_CAPACITY 64

class ReadCsvValid : public testing::Test {
protected:
  Region r;
  CSV *csv = NULL;
  void SetUp() override {
    r = region_malloc(1024 * 1024 * 50);
    csv = (CSV *)region_alloc(&r, sizeof(*csv));
    csv->capacity = TEST_CSV_CAPACITY;
    csv->rows = (Row *)region_alloc(&r, csv->capacity * sizeof(*csv->rows));
    for (size_t i = 0; i < csv->capacity; i++) {
      csv->rows[i].capacity = TEST_ROW_CAPACITY;
      csv->rows[i].fields = (Field *)region_alloc(
          &r, csv->rows[i].capacity * sizeof(*csv->rows->fields));
      csv->rows[i].size = 0;
    }
  }

  void TearDown() override { region_free(&r); }
};
