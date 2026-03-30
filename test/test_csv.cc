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

TEST_F(ReadCsvValid, AllEmptyRows) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_all_empty_rows.csv"), 4);
  EXPECT_EQ(csv->rows[0].fields[0].field_type, STRING_TYPE);
  EXPECT_EQ(csv->rows[1].fields[0].field_type, NULL_TYPE);
}

TEST_F(ReadCsvValid, BlankLinesBetweenRows) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_blank_lines_between_rows.csv"),
            6);
  EXPECT_EQ(csv->rows[0].fields[0].field_type, STRING_TYPE);
  EXPECT_EQ(csv->rows[0].fields[1].field_type, STRING_TYPE);
  EXPECT_EQ(csv->rows[1].fields[0].field_type, INT_TYPE);
  EXPECT_EQ(csv->rows[1].fields[1].field_type, STRING_TYPE);
  EXPECT_EQ(csv->rows[2].fields[0].field_type, NULL_TYPE);
}

TEST_F(ReadCsvValid, BooleanLikeValues) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_boolean_like_values.csv"), 5);
  EXPECT_EQ(csv->rows[0].fields[0].field_type, STRING_TYPE);
  EXPECT_EQ(csv->rows[1].fields[0].field_type, BOOLEAN_TYPE);
  EXPECT_EQ(csv->rows[2].fields[0].field_type, BOOLEAN_TYPE);
  EXPECT_EQ(csv->rows[3].fields[0].field_type, BOOLEAN_TYPE);
  EXPECT_EQ(csv->rows[4].fields[0].field_type, BOOLEAN_TYPE);
}

TEST_F(ReadCsvValid, CarriageReturnLineEndings) {
  EXPECT_EQ(
      read_csv(&r, csv, "./csv_valid/valid_carriage_return_line_endings.csv"),
      3);
  EXPECT_EQ(csv->rows[0].fields[0].field_type, STRING_TYPE);
  EXPECT_EQ(csv->rows[0].fields[1].field_type, STRING_TYPE);
  EXPECT_EQ(csv->rows[1].fields[0].field_type, INT_TYPE);
  EXPECT_EQ(csv->rows[1].fields[1].field_type, STRING_TYPE);
  EXPECT_EQ(csv->rows[2].fields[0].field_type, INT_TYPE);
  EXPECT_EQ(csv->rows[2].fields[1].field_type, STRING_TYPE);
}
