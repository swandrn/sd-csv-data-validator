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
  EXPECT_EQ(csv->rows[0].size, 3);
  EXPECT_EQ(csv->rows[1].size, 3);
  EXPECT_EQ(csv->rows[2].size, 3);
  EXPECT_EQ(csv->rows[3].size, 3);
}

TEST_F(ReadCsvValid, BlankLinesBetweenRows) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_blank_lines_between_rows.csv"),
            6);
  EXPECT_EQ(csv->rows[0].size, 2);
  EXPECT_EQ(csv->rows[1].size, 2);
  EXPECT_EQ(csv->rows[2].size, 1);
  EXPECT_EQ(csv->rows[3].size, 2);
  EXPECT_EQ(csv->rows[4].size, 1);
  EXPECT_EQ(csv->rows[5].size, 2);
}

TEST_F(ReadCsvValid, BooleanLikeValues) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_boolean_like_values.csv"), 5);
  EXPECT_EQ(csv->rows[0].size, 1);
  EXPECT_EQ(csv->rows[1].size, 1);
  EXPECT_EQ(csv->rows[2].size, 1);
  EXPECT_EQ(csv->rows[3].size, 1);
  EXPECT_EQ(csv->rows[4].size, 1);
}

TEST_F(ReadCsvValid, CarriageReturnLineEndings) {
  EXPECT_EQ(
      read_csv(&r, csv, "./csv_valid/valid_carriage_return_line_endings.csv"),
      3);
  EXPECT_EQ(csv->rows[0].size, 2);
  EXPECT_EQ(csv->rows[1].size, 2);
  EXPECT_EQ(csv->rows[2].size, 2);
}

TEST_F(ReadCsvValid, ColumnWithMixedTypes) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_column_with_mixed_types.csv"),
            5);
  EXPECT_EQ(csv->rows[0].size, 1);
  EXPECT_EQ(csv->rows[1].size, 1);
  EXPECT_EQ(csv->rows[2].size, 1);
  EXPECT_EQ(csv->rows[3].size, 1);
  EXPECT_EQ(csv->rows[4].size, 1);
}

TEST_F(ReadCsvValid, CommaOnlyField) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_comma_only_field.csv"), 3);
  EXPECT_EQ(csv->rows[0].size, 1);
  EXPECT_EQ(csv->rows[1].size, 1);
  EXPECT_EQ(csv->rows[2].size, 1);
}

TEST_F(ReadCsvValid, CommentLikeTextAsData) {
  EXPECT_EQ(
      read_csv(&r, csv, "./csv_valid/valid_comment_like_text_as_data.csv"), 4);
  EXPECT_EQ(csv->rows[0].size, 1);
  EXPECT_EQ(csv->rows[1].size, 1);
  EXPECT_EQ(csv->rows[2].size, 1);
  EXPECT_EQ(csv->rows[3].size, 1);
}

TEST_F(ReadCsvValid, CRLFLineEndings) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_crlf_line_endings.csv"), 3);
  EXPECT_EQ(csv->rows[0].size, 2);
  EXPECT_EQ(csv->rows[1].size, 2);
  EXPECT_EQ(csv->rows[2].size, 2);
}

TEST_F(ReadCsvValid, CurrencyLikeValues) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_currency_like_values.csv"), 4);
  EXPECT_EQ(csv->rows[0].size, 1);
  EXPECT_EQ(csv->rows[1].size, 1);
  EXPECT_EQ(csv->rows[2].size, 1);
  EXPECT_EQ(csv->rows[3].size, 1);
}

TEST_F(ReadCsvValid, DateLikeValues) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_date_like_values.csv"), 3);
  EXPECT_EQ(csv->rows[0].size, 1);
  EXPECT_EQ(csv->rows[1].size, 1);
  EXPECT_EQ(csv->rows[2].size, 1);
}

TEST_F(ReadCsvValid, DatetimeLikeValues) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_datetime_like_values.csv"), 3);
  EXPECT_EQ(csv->rows[0].size, 1);
  EXPECT_EQ(csv->rows[1].size, 1);
  EXPECT_EQ(csv->rows[2].size, 1);
}

TEST_F(ReadCsvValid, DelimiterInsideQuotes) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_delimiter_inside_quotes.csv"),
            3);
  EXPECT_EQ(csv->rows[0].size, 2);
  EXPECT_EQ(csv->rows[1].size, 2);
  EXPECT_EQ(csv->rows[2].size, 2);
}

TEST_F(ReadCsvValid, DuplicateHeaderNames) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_duplicate_header_names.csv"),
            3);
  EXPECT_EQ(csv->rows[0].size, 3);
  EXPECT_EQ(csv->rows[1].size, 3);
  EXPECT_EQ(csv->rows[2].size, 3);
}

TEST_F(ReadCsvValid, EmailLikeStrings) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_email_like_strings.csv"), 3);
  EXPECT_EQ(csv->rows[0].size, 1);
  EXPECT_EQ(csv->rows[1].size, 1);
  EXPECT_EQ(csv->rows[2].size, 1);
}

TEST_F(ReadCsvValid, EmptyFields) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_empty_fields.csv"), 4);
  EXPECT_EQ(csv->rows[0].size, 4);
  EXPECT_EQ(csv->rows[1].size, 4);
  EXPECT_EQ(csv->rows[2].size, 4);
  EXPECT_EQ(csv->rows[3].size, 4);
}

TEST_F(ReadCsvValid, EmptyFile) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_empty_file.csv"), 0);
  EXPECT_EQ(csv->size, 0);
}

TEST_F(ReadCsvValid, FloatsOnly) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_floats_only.csv"), 3);
  EXPECT_EQ(csv->rows[0].size, 2);
  EXPECT_EQ(csv->rows[1].size, 2);
  EXPECT_EQ(csv->rows[2].size, 2);
}

TEST_F(ReadCsvValid, IntegersOnly) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_integers_only.csv"), 3);
  EXPECT_EQ(csv->rows[0].size, 3);
  EXPECT_EQ(csv->rows[1].size, 3);
  EXPECT_EQ(csv->rows[2].size, 3);
}

TEST_F(ReadCsvValid, JsonLikeStrings) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_json_like_strings.csv"), 3);
  EXPECT_EQ(csv->rows[0].size, 1);
  EXPECT_EQ(csv->rows[1].size, 1);
  EXPECT_EQ(csv->rows[2].size, 1);
}

TEST_F(ReadCsvValid, LineFeedLineEndings) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_line_feed_line_endings.csv"),
            3);
  EXPECT_EQ(csv->rows[0].size, 2);
  EXPECT_EQ(csv->rows[1].size, 2);
  EXPECT_EQ(csv->rows[2].size, 2);
}

TEST_F(ReadCsvValid, LongTextFields) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_long_text_fields.csv"), 3);
  EXPECT_EQ(csv->rows[0].size, 2);
  EXPECT_EQ(csv->rows[1].size, 2);
  EXPECT_EQ(csv->rows[2].size, 2);
}

TEST_F(ReadCsvValid, ManyColumns) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_many_columns.csv"), 3);
  EXPECT_EQ(csv->rows[0].size, 32);
  EXPECT_EQ(csv->rows[1].size, 32);
  EXPECT_EQ(csv->rows[2].size, 32);
}

TEST_F(ReadCsvValid, MixedEmptyAndNonEmptyRows) {
  EXPECT_EQ(
      read_csv(&r, csv, "./csv_valid/valid_mixed_empty_and_nonempty_rows.csv"),
      6);
  EXPECT_EQ(csv->rows[0].size, 3);
  EXPECT_EQ(csv->rows[1].size, 3);
  EXPECT_EQ(csv->rows[2].size, 3);
  EXPECT_EQ(csv->rows[3].size, 3);
  EXPECT_EQ(csv->rows[4].size, 3);
  EXPECT_EQ(csv->rows[5].size, 3);
}

TEST_F(ReadCsvValid, MixedNumericAndStringValues) {
  EXPECT_EQ(read_csv(&r, csv,
                     "./csv_valid/valid_mixed_numeric_and_string_columns.csv"),
            5);
  EXPECT_EQ(csv->rows[0].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[1].fields[0].field_type, INT_FIELD);
  EXPECT_EQ(csv->rows[2].fields[0].field_type, STRING_FIELD);
  EXPECT_EQ(csv->rows[3].fields[0].field_type, FLOAT_FIELD);
  EXPECT_EQ(csv->rows[4].fields[0].field_type, STRING_FIELD);
}

TEST_F(ReadCsvValid, MultilineQuotedFields) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_multiline_quoted_fields.csv"),
            3);
  EXPECT_EQ(csv->rows[0].size, 2);
  EXPECT_EQ(csv->rows[1].size, 2);
  EXPECT_EQ(csv->rows[2].size, 2);
}

TEST_F(ReadCsvValid, NegativeNumbers) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_negative_numbers.csv"), 3);
  EXPECT_EQ(csv->rows[0].size, 2);
  EXPECT_EQ(csv->rows[1].size, 2);
  EXPECT_EQ(csv->rows[2].size, 2);
}

TEST_F(ReadCsvValid, NullLikeLiterals) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_null_like_literals.csv"), 5);
  EXPECT_EQ(csv->rows[0].size, 1);
  EXPECT_EQ(csv->rows[1].size, 1);
  EXPECT_EQ(csv->rows[2].size, 1);
  EXPECT_EQ(csv->rows[3].size, 1);
  EXPECT_EQ(csv->rows[4].size, 1);
}

TEST_F(ReadCsvValid, PathLikeStrings) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_path_like_strings.csv"), 3);
  EXPECT_EQ(csv->rows[0].size, 1);
  EXPECT_EQ(csv->rows[1].size, 1);
  EXPECT_EQ(csv->rows[2].size, 1);
}

TEST_F(ReadCsvValid, PercentageLikeValues) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_percentage_like_values.csv"),
            4);
  EXPECT_EQ(csv->rows[0].size, 1);
  EXPECT_EQ(csv->rows[1].size, 1);
  EXPECT_EQ(csv->rows[2].size, 1);
  EXPECT_EQ(csv->rows[3].size, 1);
}

TEST_F(ReadCsvValid, PhoneNumberLikeStrings) {
  EXPECT_EQ(
      read_csv(&r, csv, "./csv_valid/valid_phone_number_like_strings.csv"), 3);
  EXPECT_EQ(csv->rows[0].size, 1);
  EXPECT_EQ(csv->rows[1].size, 1);
  EXPECT_EQ(csv->rows[2].size, 1);
}

TEST_F(ReadCsvValid, QuoteOnlyField) {
  EXPECT_EQ(read_csv(&r, csv, "./csv_valid/valid_quote_only_field.csv"), 3);
  EXPECT_EQ(csv->rows[0].size, 1);
  EXPECT_EQ(csv->rows[1].size, 1);
  EXPECT_EQ(csv->rows[2].size, 1);
}

TEST_F(ReadCsvValid, QuotedFieldsWithEscapedQuotes) {
  EXPECT_EQ(read_csv(&r, csv,
                     "./csv_valid/valid_quoted_fields_with_escaped_quotes.csv"),
            3);
  EXPECT_EQ(csv->rows[0].size, 2);
  EXPECT_EQ(csv->rows[1].size, 2);
  EXPECT_EQ(csv->rows[2].size, 2);
}
