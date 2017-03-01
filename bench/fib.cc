#include <iostream>

int main() {
  int base1 = 0;
  int base2 = 1;
  int nth = 200000;
  int temp;
  while (nth-- > 0) {
    temp = base2;
    base2 = base1 + base2;
    base1 = temp;
  }
  std::cout << base1 << std::endl;
}
