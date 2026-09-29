#include "json/Json.hpp"
#include "test_framework.hpp"

using returns::json::ParseError;
using returns::json::TypeError;
using returns::json::Value;
using returns::json::parse;

TEST(Json, ParsesScalars) {
  EXPECT_TRUE(parse("null").isNull());
  EXPECT_EQ(parse("true").asBool(), true);
  EXPECT_EQ(parse("false").asBool(), false);
  EXPECT_EQ(parse("42").asInt(), 42);
  EXPECT_EQ(parse("-7").asInt(), -7);
  EXPECT_EQ(parse("2.5").asNumber(), 2.5);
  EXPECT_EQ(parse("1e3").asNumber(), 1000.0);
  EXPECT_EQ(parse("\"hi\"").asString(), std::string("hi"));
}

TEST(Json, ParsesNestedStructures) {
  const Value v = parse("{\"a\": [1, 2, {\"b\": null}], \"c\": {\"d\": \"e\"}, \"f\": []}");
  EXPECT_EQ(v["a"].asArray().size(), static_cast<std::size_t>(3));
  EXPECT_EQ(v["a"].asArray()[1].asInt(), 2);
  EXPECT_TRUE(v["a"].asArray()[2]["b"].isNull());
  EXPECT_EQ(v["c"]["d"].asString(), std::string("e"));
  EXPECT_TRUE(v["f"].asArray().empty());
}

TEST(Json, SimpleEscapes) {
  EXPECT_EQ(parse(R"("a\"b\\c\/d")").asString(), std::string("a\"b\\c/d"));
  EXPECT_EQ(parse(R"("line1\nline2\ttab")").asString(), std::string("line1\nline2\ttab"));
}

TEST(Json, ToleratesAllJsonWhitespace) {
  const Value v = parse("{\r\n\t\"items\": [1,\t2 ,\r\n 3]\r\n}\r\n");
  EXPECT_EQ(v["items"].asArray().size(), static_cast<std::size_t>(3));
}

// Note: these use "\\u" (not raw strings) so the JSON text really contains a backslash-u escape.

TEST(Json, UnicodeEscapeAscii) {
  EXPECT_EQ(parse("\"\\u0041\\u0062\"").asString(), std::string("Ab"));
}

TEST(Json, UnicodeEscapeTwoByteUtf8) {
  EXPECT_EQ(parse("\"Jos\\u00e9\"").asString(), std::string("Jos\xC3\xA9"));  // é
  EXPECT_EQ(parse("\"\\u00c5\"").asString(), std::string("\xC3\x85"));        // Å
}

TEST(Json, UnicodeEscapeThreeAndFourByteUtf8) {
  EXPECT_EQ(parse("\"\\u20ac\"").asString(), std::string("\xE2\x82\xAC"));              // €
  EXPECT_EQ(parse("\"\\ud83d\\ude00\"").asString(), std::string("\xF0\x9F\x98\x80"));  // emoji
}

TEST(Json, RejectsMalformedDocuments) {
  EXPECT_THROW(parse(""), ParseError);
  EXPECT_THROW(parse("{\"a\" 1}"), ParseError);
  EXPECT_THROW(parse("[1, 2"), ParseError);
  EXPECT_THROW(parse("\"unterminated"), ParseError);
  EXPECT_THROW(parse("tru"), ParseError);
  EXPECT_THROW(parse("{} extra"), ParseError);
  EXPECT_THROW(parse("01"), ParseError);
}

TEST(Json, TypedAccessors) {
  const Value v = parse("{\"n\": 1.5, \"s\": \"x\"}");
  EXPECT_THROW(v["n"].asInt(), TypeError);
  EXPECT_THROW(v["s"].asNumber(), TypeError);
  EXPECT_THROW(v["missing"], std::out_of_range);
  EXPECT_EQ(v.getString("missing", "fallback"), std::string("fallback"));
  EXPECT_EQ(v.getInt("missing", 9), 9);
}
