#include <fmt/core.h>

int main() {
  auto lang = "C++";
  fmt::println("Hello and welcome to {}", lang);

  for (int i = 1; i <= 5; i++) {
    fmt::println("i = {}", i);
  }

  return 0;
}