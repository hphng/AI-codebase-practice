// Usage: unit_tests [FILTER]
// Runs every test whose "Suite.Name" contains FILTER (all tests if omitted).

#include "test_framework.hpp"

int main(int argc, char** argv) {
  return tf::runAll(argc > 1 ? argv[1] : "");
}
