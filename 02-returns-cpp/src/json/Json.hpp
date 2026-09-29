#pragma once

// Minimal JSON (RFC 8259) reader used for config and data files.

#include <cstddef>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace returns::json {

class ParseError : public std::runtime_error {
 public:
  ParseError(const std::string& message, std::size_t position)
      : std::runtime_error(message), position_(position) {}
  std::size_t position() const { return position_; }

 private:
  std::size_t position_;
};

class TypeError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

class Value {
 public:
  enum class Type { Null, Bool, Number, String, Array, Object };
  using Array = std::vector<Value>;
  using Object = std::map<std::string, Value>;

  Value() = default;
  Value(std::nullptr_t) {}
  Value(bool b) : data_(b) {}
  Value(double n) : data_(n) {}
  Value(int n) : data_(static_cast<double>(n)) {}
  Value(const char* s) : data_(std::string(s)) {}
  Value(std::string s) : data_(std::move(s)) {}
  Value(Array a) : data_(std::move(a)) {}
  Value(Object o) : data_(std::move(o)) {}

  Type type() const { return static_cast<Type>(data_.index()); }
  bool isNull() const { return type() == Type::Null; }

  bool asBool() const;
  double asNumber() const;
  std::int64_t asInt() const;  // throws TypeError if the number is not integral
  const std::string& asString() const;
  const Array& asArray() const;
  const Object& asObject() const;

  // Object access. operator[] throws std::out_of_range if the key is missing.
  bool has(const std::string& key) const;
  const Value& operator[](const std::string& key) const;

  // Object access with a fallback when the key is missing.
  std::string getString(const std::string& key, const std::string& fallback) const;
  std::int64_t getInt(const std::string& key, std::int64_t fallback) const;
  bool getBool(const std::string& key, bool fallback) const;

 private:
  std::variant<std::nullptr_t, bool, double, std::string, Array, Object> data_{nullptr};
};

// Parses a complete JSON document. Throws ParseError.
Value parse(const std::string& text);

// Reads and parses a file (UTF-8, optional BOM). Throws std::runtime_error / ParseError.
Value parseFile(const std::string& path);

}  // namespace returns::json
