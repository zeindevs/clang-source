#include <iostream>

template <typename T, typename U = double> U average(const T &a, const T &b) {
  return (a + b) / U{2};
}

int main() {
  std::cout << "Hello Bro!" << std::endl;
  std::cout << "average: " << average(3.5, 3.0) << std::endl;
  return 0;
}
