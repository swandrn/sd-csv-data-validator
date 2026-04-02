#include <gtest/gtest.h>
extern "C" {
#include "csv.h"
}

TEST(CsvGetfield, AlphabeticalValues) {
  char buf[] = "a,b,c";
  char *p = buf;

  EXPECT_STREQ(csv_getfield(&p), "a");
  EXPECT_STREQ(csv_getfield(&p), "b");
  EXPECT_STREQ(csv_getfield(&p), "c");
  EXPECT_EQ(csv_getfield(&p), nullptr);
}

TEST(CsvGetfield, NumericalValues) {
  char buf[] = "1,2,3";
  char *p = buf;

  EXPECT_STREQ(csv_getfield(&p), "1");
  EXPECT_STREQ(csv_getfield(&p), "2");
  EXPECT_STREQ(csv_getfield(&p), "3");
  EXPECT_EQ(csv_getfield(&p), nullptr);
}

TEST(CsvGetfield, SecondFieldQuoted) {
  char buf[] = "alpha,\"beta,gamma\"";
  char *p = buf;

  EXPECT_STREQ(csv_getfield(&p), "alpha");
  EXPECT_STREQ(csv_getfield(&p), "beta,gamma");
  EXPECT_EQ(csv_getfield(&p), nullptr);
}

TEST(CsvGetfield, FirstFieldQuoted) {
  char buf[] = "\"alpha,beta\",gamma";
  char *p = buf;

  EXPECT_STREQ(csv_getfield(&p), "alpha,beta");
  EXPECT_STREQ(csv_getfield(&p), "gamma");
  EXPECT_EQ(csv_getfield(&p), nullptr);
}

TEST(CsvGetfield, TwoEmptyStrings) {
  char buf[] = "\"\",\"\"";
  char *p = buf;

  EXPECT_STREQ(csv_getfield(&p), "");
  EXPECT_STREQ(csv_getfield(&p), "");
  EXPECT_EQ(csv_getfield(&p), nullptr);
}

TEST(CsvGetfield, TwoEmptyStringsAndAValue) {
  char buf[] = "\"\",\"\",a";
  char *p = buf;

  EXPECT_STREQ(csv_getfield(&p), "");
  EXPECT_STREQ(csv_getfield(&p), "");
  EXPECT_STREQ(csv_getfield(&p), "a");
  EXPECT_EQ(csv_getfield(&p), nullptr);
}

TEST(CsvGetfield, NestedQuotes) {
  char buf[] = "x,\"with \"\"quote\"\" inside\",y";
  char *p = buf;

  EXPECT_STREQ(csv_getfield(&p), "x");
  EXPECT_STREQ(csv_getfield(&p), "with \"quote\" inside");
  EXPECT_STREQ(csv_getfield(&p), "y");
  EXPECT_EQ(csv_getfield(&p), nullptr);
}

TEST(CsvGetfield, LeadingAndTrailingSpace) {
  char buf[] = " leading space,middle,trailing space ";
  char *p = buf;

  EXPECT_STREQ(csv_getfield(&p), " leading space");
  EXPECT_STREQ(csv_getfield(&p), "middle");
  EXPECT_STREQ(csv_getfield(&p), "trailing space ");
  EXPECT_EQ(csv_getfield(&p), nullptr);
}

TEST(CsvGetfield, LineBreak) {
  char buf[] = "line\nbreak,x";
  char *p = buf;

  EXPECT_STREQ(csv_getfield(&p), "line\nbreak");
  EXPECT_STREQ(csv_getfield(&p), "x");
  EXPECT_EQ(csv_getfield(&p), nullptr);
}

TEST(CsvGetfield, AllEmptyRows) {
  char buf[] = ",,";
  char *p = buf;

  EXPECT_STREQ(csv_getfield(&p), "\0");
  EXPECT_STREQ(csv_getfield(&p), "\0");
  EXPECT_STREQ(csv_getfield(&p), "\0");
  EXPECT_EQ(csv_getfield(&p), nullptr);
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
  EXPECT_EQ(csv->rows[0].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[1].fields[0].field_type, NULL_FIELD);
}

TEST_F(ReadCsvValid, BlankLinesBetweenRows) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_blank_lines_between_rows.csv"),
            6);
  EXPECT_EQ(csv->rows[0].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[0].fields[1].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[1].fields[0].field_type, INT_FIELD);
  EXPECT_EQ(csv->rows[1].fields[1].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[2].fields[0].field_type, NULL_FIELD);
}

TEST_F(ReadCsvValid, BooleanLikeValues) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_boolean_like_values.csv"), 5);
  EXPECT_EQ(csv->rows[0].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[1].fields[0].field_type, BOOLEAN_FIELD);
  EXPECT_EQ(csv->rows[2].fields[0].field_type, BOOLEAN_FIELD);
  EXPECT_EQ(csv->rows[3].fields[0].field_type, BOOLEAN_FIELD);
  EXPECT_EQ(csv->rows[4].fields[0].field_type, BOOLEAN_FIELD);
}

TEST_F(ReadCsvValid, CarriageReturnLineEndings) {
  EXPECT_EQ(
      read_csv(&r, csv, "./csv_valid/valid_carriage_return_line_endings.csv"),
      3);
  EXPECT_EQ(csv->rows[0].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[0].fields[1].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[1].fields[0].field_type, INT_FIELD);
  EXPECT_EQ(csv->rows[1].fields[1].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[2].fields[0].field_type, INT_FIELD);
  EXPECT_EQ(csv->rows[2].fields[1].field_type, STRING_FIELD);
}

TEST_F(ReadCsvValid, ColumnWithMixedTypes) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_column_with_mixed_types.csv"),
            5);
  EXPECT_EQ(csv->rows[0].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[1].fields[0].field_type, INT_FIELD);
  EXPECT_EQ(csv->rows[2].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[3].fields[0].field_type, FLOAT_FIELD);
  EXPECT_EQ(csv->rows[4].fields[0].field_type, NULL_FIELD);
}

TEST_F(ReadCsvValid, CommaOnlyField) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_comma_only_field.csv"), 3);
  EXPECT_EQ(csv->rows[0].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[1].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[2].fields[0].field_type, STRING_FIELD);
}

TEST_F(ReadCsvValid, CommentLikeTextAsData) {
  EXPECT_EQ(
      read_csv(&r, csv, "./csv_valid/valid_comment_like_text_as_data.csv"), 4);
  EXPECT_EQ(csv->rows[0].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[1].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[2].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[3].fields[0].field_type, STRING_FIELD);
}

TEST_F(ReadCsvValid, CRLFLineEndings) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_crlf_line_endings.csv"), 3);
  EXPECT_EQ(csv->rows[0].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[0].fields[1].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[1].fields[0].field_type, INT_FIELD);
  EXPECT_EQ(csv->rows[1].fields[1].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[2].fields[0].field_type, INT_FIELD);
  EXPECT_EQ(csv->rows[2].fields[1].field_type, STRING_FIELD);
}

TEST_F(ReadCsvValid, CurrencyLikeValues) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_currency_like_values.csv"), 4);
  EXPECT_EQ(csv->rows[0].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[1].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[2].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[3].fields[0].field_type, STRING_FIELD);
}

TEST_F(ReadCsvValid, DateLikeValues) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_date_like_values.csv"), 3);
  EXPECT_EQ(csv->rows[0].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[1].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[2].fields[0].field_type, STRING_FIELD);
}

TEST_F(ReadCsvValid, DatetimeLikeValues) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_datetime_like_values.csv"), 3);
  EXPECT_EQ(csv->rows[0].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[1].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[2].fields[0].field_type, STRING_FIELD);
}

TEST_F(ReadCsvValid, DelimiterInsideQuotes) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_delimiter_inside_quotes.csv"),
            3);
  EXPECT_EQ(csv->rows[0].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[1].fields[0].field_type, INT_FIELD);
  EXPECT_EQ(csv->rows[1].fields[1].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[2].fields[0].field_type, INT_FIELD);
  EXPECT_EQ(csv->rows[2].fields[1].field_type, STRING_FIELD);
}

TEST_F(ReadCsvValid, DuplicateHeaderNames) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_duplicate_header_names.csv"),
            3);
  EXPECT_EQ(csv->rows[0].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[1].fields[0].field_type, INT_FIELD);
  EXPECT_EQ(csv->rows[1].fields[1].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[1].fields[2].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[2].fields[0].field_type, INT_FIELD);
  EXPECT_EQ(csv->rows[2].fields[1].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[2].fields[2].field_type, STRING_FIELD);
}

TEST_F(ReadCsvValid, EmailLikeStrings) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_email_like_strings.csv"), 3);
  EXPECT_EQ(csv->rows[0].size, 1);
  EXPECT_EQ(csv->rows[0].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[1].size, 1);
  EXPECT_EQ(csv->rows[1].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[2].size, 1);
  EXPECT_EQ(csv->rows[2].fields[0].field_type, STRING_FIELD);
}

TEST_F(ReadCsvValid, EmptyFields) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_empty_fields.csv"), 4);
  EXPECT_EQ(csv->rows[0].size, 4);
  EXPECT_EQ(csv->rows[1].size, 4);
  EXPECT_EQ(csv->rows[1].fields[0].field_type, INT_FIELD);
  EXPECT_EQ(csv->rows[1].fields[1].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[1].fields[2].field_type, NULL_FIELD);
  EXPECT_EQ(csv->rows[1].fields[3].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[2].size, 4);
  EXPECT_EQ(csv->rows[2].fields[0].field_type, INT_FIELD);
  EXPECT_EQ(csv->rows[2].fields[1].field_type, NULL_FIELD);
  EXPECT_EQ(csv->rows[2].fields[2].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[2].fields[3].field_type, NULL_FIELD);
  EXPECT_EQ(csv->rows[3].size, 4);
  EXPECT_EQ(csv->rows[3].fields[0].field_type, INT_FIELD);
  EXPECT_EQ(csv->rows[3].fields[1].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[3].fields[2].field_type, NULL_FIELD);
  EXPECT_EQ(csv->rows[3].fields[3].field_type, NULL_FIELD);
}

TEST_F(ReadCsvValid, EmptyFile) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_empty_file.csv"), 0);
  EXPECT_EQ(csv->size, 0);
}

TEST_F(ReadCsvValid, FloatsOnly) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_floats_only.csv"), 3);
  EXPECT_EQ(csv->rows[0].size, 2);
  EXPECT_EQ(csv->rows[0].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[1].size, 2);
  EXPECT_EQ(csv->rows[1].fields[0].field_type, FLOAT_FIELD);
  EXPECT_EQ(csv->rows[2].size, 2);
  EXPECT_EQ(csv->rows[2].fields[0].field_type, FLOAT_FIELD);
}
