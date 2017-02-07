#include <iostream>
#include "src/main/grammar/stg.tab.hh"
#include "lib/ast.hh"

int main() {
  Prog* p;
  yy::stg_parser parser(*p);
  int v = parser.parse();
  return v;
}
