#include "json/Json.hpp"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace returns::json {

namespace {

const char* typeName(Value::Type t) {
  switch (t) {
    case Value::Type::Null: return "null";
    case Value::Type::Bool: return "bool";
    case Value::Type::Number: return "number";
    case Value::Type::String: return "string";
    case Value::Type::Array: return "array";
    case Value::Type::Object: return "object";
  }
  return "?";
}

[[noreturn]] void typeError(const char* expected, Value::Type actual) {
  throw TypeError(std::string("expected ") + expected + ", got " + typeName(actual));
}

class Parser {
 public:
  explicit Parser(const std::string& text) : text_(text) {}

  Value parseDocument() {
    skipWhitespace();
    Value value = parseValue();
    skipWhitespace();
    if (!atEnd()) fail("unexpected trailing characters");
    return value;
  }

 private:
  const std::string& text_;
  std::size_t pos_ = 0;

  [[noreturn]] void fail(const std::string& message) const {
    throw ParseError(message + " at position " + std::to_string(pos_), pos_);
  }

  bool atEnd() const { return pos_ >= text_.size(); }
  char peek() const { return atEnd() ? '\0' : text_[pos_]; }

  char next() {
    if (atEnd()) fail("unexpected end of input");
    return text_[pos_++];
  }

  void expect(char c) {
    if (next() != c) {
      --pos_;
      fail(std::string("expected '") + c + "'");
    }
  }

  void skipWhitespace() {
    while (!atEnd() && (text_[pos_] == ' ' || text_[pos_] == '\n' || text_[pos_] == '\r' || text_[pos_] == '\t')) ++pos_;
  }

  static bool isDigit(char c) { return std::isdigit(static_cast<unsigned char>(c)) != 0; }

  Value parseValue() {
    const char c = peek();
    switch (c) {
      case '{': return parseObject();
      case '[': return parseArray();
      case '"': return Value(parseString());
      case 't': return parseLiteral("true", Value(true));
      case 'f': return parseLiteral("false", Value(false));
      case 'n': return parseLiteral("null", Value(nullptr));
      default:
        if (c == '-' || isDigit(c)) return parseNumber();
        fail(atEnd() ? "unexpected end of input" : std::string("unexpected character '") + c + "'");
    }
  }

  Value parseLiteral(const char* word, Value value) {
    for (const char* p = word; *p != '\0'; ++p) {
      if (next() != *p) {
        --pos_;
        fail(std::string("invalid literal, expected ") + word);
      }
    }
    return value;
  }

  Value parseObject() {
    expect('{');
    Value::Object object;
    skipWhitespace();
    if (peek() == '}') {
      ++pos_;
      return Value(std::move(object));
    }
    while (true) {
      skipWhitespace();
      if (peek() != '"') fail("expected string key");
      std::string key = parseString();
      skipWhitespace();
      expect(':');
      skipWhitespace();
      object[key] = parseValue();
      skipWhitespace();
      const char c = next();
      if (c == '}') break;
      if (c != ',') {
        --pos_;
        fail("expected ',' or '}'");
      }
    }
    return Value(std::move(object));
  }

  Value parseArray() {
    expect('[');
    Value::Array array;
    skipWhitespace();
    if (peek() == ']') {
      ++pos_;
      return Value(std::move(array));
    }
    while (true) {
      skipWhitespace();
      array.push_back(parseValue());
      skipWhitespace();
      const char c = next();
      if (c == ']') break;
      if (c != ',') {
        --pos_;
        fail("expected ',' or ']'");
      }
    }
    return Value(std::move(array));
  }

  Value parseNumber() {
    const std::size_t start = pos_;
    if (peek() == '-') ++pos_;
    if (!isDigit(peek())) fail("invalid number");
    if (peek() == '0') {
      ++pos_;
    } else {
      while (isDigit(peek())) ++pos_;
    }
    if (peek() == '.') {
      ++pos_;
      if (!isDigit(peek())) fail("expected digit after decimal point");
      while (isDigit(peek())) ++pos_;
    }
    if (peek() == 'e' || peek() == 'E') {
      ++pos_;
      if (peek() == '+' || peek() == '-') ++pos_;
      if (!isDigit(peek())) fail("expected exponent digits");
      while (isDigit(peek())) ++pos_;
    }
    return Value(std::strtod(text_.substr(start, pos_ - start).c_str(), nullptr));
  }

  std::uint32_t parseHex4() {
    std::uint32_t value = 0;
    for (int i = 0; i < 4; ++i) {
      const char c = next();
      value <<= 4;
      if (c >= '0' && c <= '9') value |= static_cast<std::uint32_t>(c - '0');
      else if (c >= 'a' && c <= 'f') value |= static_cast<std::uint32_t>(c - 'a' + 10);
      else if (c >= 'A' && c <= 'F') value |= static_cast<std::uint32_t>(c - 'A' + 10);
      else {
        --pos_;
        fail("invalid \\u escape");
      }
    }
    return value;
  }

  // Encodes a Unicode code point as UTF-8.
  static void appendUtf8(std::string& out, std::uint32_t cp) {
    if (cp < 0x80) {
      out += static_cast<char>(cp);
    } else if (cp < 0x800) {
      out += static_cast<char>(0xC0 | (cp >> 6));
      out += static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
      out += static_cast<char>(0xE0 | (cp >> 12));
      out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
      out += static_cast<char>(0x80 | (cp & 0x3F));
    } else {
      out += static_cast<char>(0xF0 | (cp >> 18));
      out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
      out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
      out += static_cast<char>(0x80 | (cp & 0x3F));
    }
  }

  std::string parseString() {
    expect('"');
    std::string out;
    while (true) {
      const char c = next();
      if (c == '"') break;
      if (static_cast<unsigned char>(c) < 0x20) {
        --pos_;
        fail("unescaped control character in string");
      }
      if (c != '\\') {
        out += c;
        continue;
      }
      const char e = next();
      switch (e) {
        case '"': out += '"'; break;
        case '\\': out += '\\'; break;
        case '/': out += '/'; break;
        case 'b': out += '\b'; break;
        case 'f': out += '\f'; break;
        case 'n': out += '\n'; break;
        case 'r': out += '\r'; break;
        case 't': out += '\t'; break;
        case 'u': {
          std::uint32_t cp = parseHex4();
          if (cp >= 0xD800 && cp <= 0xDBFF) {  // high surrogate: must be followed by a low one
            if (next() != '\\' || next() != 'u') fail("unpaired surrogate");
            const std::uint32_t low = parseHex4();
            if (low < 0xDC00 || low > 0xDFFF) fail("invalid low surrogate");
            cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
          } else if (cp >= 0xDC00 && cp <= 0xDFFF) {
            fail("unpaired surrogate");
          }
          appendUtf8(out, cp);
          break;
        }
        default:
          --pos_;
          fail(std::string("invalid escape '\\") + e + "'");
      }
    }
    return out;
  }
};

}  // namespace

bool Value::asBool() const {
  if (auto p = std::get_if<bool>(&data_)) return *p;
  typeError("bool", type());
}

double Value::asNumber() const {
  if (auto p = std::get_if<double>(&data_)) return *p;
  typeError("number", type());
}

std::int64_t Value::asInt() const {
  const double n = asNumber();
  if (std::floor(n) != n) throw TypeError("expected an integer, got " + std::to_string(n));
  return static_cast<std::int64_t>(n);
}

const std::string& Value::asString() const {
  if (auto p = std::get_if<std::string>(&data_)) return *p;
  typeError("string", type());
}

const Value::Array& Value::asArray() const {
  if (auto p = std::get_if<Array>(&data_)) return *p;
  typeError("array", type());
}

const Value::Object& Value::asObject() const {
  if (auto p = std::get_if<Object>(&data_)) return *p;
  typeError("object", type());
}

bool Value::has(const std::string& key) const {
  const auto& object = asObject();
  return object.find(key) != object.end();
}

const Value& Value::operator[](const std::string& key) const {
  const auto& object = asObject();
  auto it = object.find(key);
  if (it == object.end()) throw std::out_of_range("missing key \"" + key + "\"");
  return it->second;
}

std::string Value::getString(const std::string& key, const std::string& fallback) const {
  return has(key) ? (*this)[key].asString() : fallback;
}

std::int64_t Value::getInt(const std::string& key, std::int64_t fallback) const {
  return has(key) ? (*this)[key].asInt() : fallback;
}

bool Value::getBool(const std::string& key, bool fallback) const {
  return has(key) ? (*this)[key].asBool() : fallback;
}

Value parse(const std::string& text) {
  return Parser(text).parseDocument();
}

Value parseFile(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) throw std::runtime_error("cannot open " + path);
  std::ostringstream buffer;
  buffer << in.rdbuf();
  std::string text = buffer.str();
  if (text.rfind("\xEF\xBB\xBF", 0) == 0) text.erase(0, 3);  // UTF-8 BOM
  try {
    return parse(text);
  } catch (const ParseError& e) {
    throw ParseError(path + ": " + e.what(), e.position());
  }
}

}  // namespace returns::json
