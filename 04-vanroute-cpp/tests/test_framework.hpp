#pragma once

// Tiny GoogleTest-style test framework (header-only, no dependencies).
//
//   TEST(Suite, Name) { EXPECT_EQ(a, b); ASSERT_TRUE(x); EXPECT_THROW(stmt, Type); ... }
//
// EXPECT_* records a failure and keeps going; ASSERT_* stops the current test.

#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace tf {

struct TestCase {
  std::string suite;
  std::string name;
  std::function<void()> body;
};

inline std::vector<TestCase>& registry() {
  static std::vector<TestCase> tests;
  return tests;
}

struct Registrar {
  Registrar(const char* suite, const char* name, std::function<void()> body) {
    registry().push_back({suite, name, std::move(body)});
  }
};

struct AssertionAbort {};

inline int& currentFailures() {
  static int count = 0;
  return count;
}

inline void reportFailure(const char* file, int line, const std::string& message) {
  std::cout << file << ":" << line << ": Failure\n" << message << "\n";
  ++currentFailures();
}

template <typename T, typename = void>
struct IsStreamable : std::false_type {};
template <typename T>
struct IsStreamable<T, std::void_t<decltype(std::declval<std::ostream&>() << std::declval<const T&>())>>
    : std::true_type {};

template <typename T>
std::string show(const T& value) {
  if constexpr (std::is_same_v<T, bool>) {
    return value ? "true" : "false";
  } else if constexpr (std::is_convertible_v<T, std::string>) {
    return "\"" + std::string(value) + "\"";
  } else if constexpr (IsStreamable<T>::value) {
    std::ostringstream out;
    out << value;
    return out.str();
  } else {
    return "<unprintable>";
  }
}

template <typename A, typename B>
bool checkEq(const A& actual, const B& expected, const char* actualExpr, const char* expectedExpr,
             const char* file, int line, bool fatal) {
  if (actual == expected) return true;
  reportFailure(file, line,
                std::string("Expected equality of these values:\n  ") + actualExpr + "\n    Which is: " +
                    show(actual) + "\n  " + expectedExpr + "\n    Which is: " + show(expected));
  if (fatal) throw AssertionAbort{};
  return false;
}

template <typename A, typename B>
bool checkNe(const A& a, const B& b, const char* aExpr, const char* bExpr, const char* file, int line) {
  if (!(a == b)) return true;
  reportFailure(file, line, std::string("Expected (") + aExpr + ") != (" + bExpr + "), both are " + show(a));
  return false;
}

inline bool checkTrue(bool value, bool expected, const char* expr, const char* file, int line, bool fatal) {
  if (value == expected) return true;
  reportFailure(file, line, std::string("Value of: ") + expr + "\n  Actual: " + show(value) +
                                "\nExpected: " + show(expected));
  if (fatal) throw AssertionAbort{};
  return false;
}

inline bool checkContains(const std::string& haystack, const std::string& needle, const char* expr,
                          const char* file, int line) {
  if (haystack.find(needle) != std::string::npos) return true;
  reportFailure(file, line, std::string("Expected ") + expr + " to contain \"" + needle +
                                "\"\n  Actual: \"" + haystack + "\"");
  return false;
}

inline int runAll(const std::string& filter) {
  int passed = 0;
  std::vector<std::string> failed;
  std::size_t selected = 0;

  for (const auto& test : registry()) {
    const std::string fullName = test.suite + "." + test.name;
    if (!filter.empty() && fullName.find(filter) == std::string::npos) continue;
    ++selected;

    std::cout << "[ RUN      ] " << fullName << std::endl;
    currentFailures() = 0;
    try {
      test.body();
    } catch (const AssertionAbort&) {
    } catch (const std::exception& e) {
      reportFailure("<unknown>", 0, std::string("Unexpected exception: ") + e.what());
    } catch (...) {
      reportFailure("<unknown>", 0, "Unexpected non-standard exception");
    }

    if (currentFailures() == 0) {
      ++passed;
      std::cout << "[       OK ] " << fullName << std::endl;
    } else {
      failed.push_back(fullName);
      std::cout << "[  FAILED  ] " << fullName << std::endl;
    }
  }

  std::cout << "\n[==========] " << selected << " tests ran.\n";
  std::cout << "[  PASSED  ] " << passed << " tests.\n";
  if (!failed.empty()) {
    std::cout << "[  FAILED  ] " << failed.size() << " tests, listed below:\n";
    for (const auto& name : failed) std::cout << "[  FAILED  ] " << name << "\n";
  }
  return failed.empty() ? 0 : 1;
}

}  // namespace tf

#define TF_CONCAT_(a, b) a##b
#define TF_CONCAT(a, b) TF_CONCAT_(a, b)

#define TEST(suite, name)                                                                   \
  static void TF_CONCAT(suite##_##name, _body)();                                           \
  static ::tf::Registrar TF_CONCAT(suite##_##name, _registrar)(#suite, #name,               \
                                                               &TF_CONCAT(suite##_##name, _body)); \
  static void TF_CONCAT(suite##_##name, _body)()

#define EXPECT_EQ(actual, expected) ::tf::checkEq((actual), (expected), #actual, #expected, __FILE__, __LINE__, false)
#define ASSERT_EQ(actual, expected) ::tf::checkEq((actual), (expected), #actual, #expected, __FILE__, __LINE__, true)
#define EXPECT_NE(a, b) ::tf::checkNe((a), (b), #a, #b, __FILE__, __LINE__)
#define EXPECT_TRUE(cond) ::tf::checkTrue(static_cast<bool>(cond), true, #cond, __FILE__, __LINE__, false)
#define EXPECT_FALSE(cond) ::tf::checkTrue(static_cast<bool>(cond), false, #cond, __FILE__, __LINE__, false)
#define ASSERT_TRUE(cond) ::tf::checkTrue(static_cast<bool>(cond), true, #cond, __FILE__, __LINE__, true)
#define EXPECT_CONTAINS(haystack, needle) ::tf::checkContains((haystack), (needle), #haystack, __FILE__, __LINE__)

#define EXPECT_THROW(statement, ExceptionType)                                                       \
  do {                                                                                               \
    bool tf_caught_ = false;                                                                         \
    try {                                                                                            \
      statement;                                                                                     \
    } catch (const ExceptionType&) {                                                                 \
      tf_caught_ = true;                                                                             \
    } catch (...) {                                                                                  \
    }                                                                                                \
    if (!tf_caught_)                                                                                 \
      ::tf::reportFailure(__FILE__, __LINE__, "Expected: " #statement " throws " #ExceptionType);  \
  } while (0)

#define EXPECT_NO_THROW(statement)                                                                   \
  do {                                                                                               \
    try {                                                                                            \
      statement;                                                                                     \
    } catch (const std::exception& tf_e_) {                                                          \
      ::tf::reportFailure(__FILE__, __LINE__,                                                        \
                          std::string("Expected: " #statement " doesn't throw. Threw: ") + tf_e_.what()); \
    }                                                                                                \
  } while (0)
