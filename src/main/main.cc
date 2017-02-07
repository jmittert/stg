#include <iostream>
#include "ast.hh"
#include "parser.hh"

int main() {
  Prog p = Prog(new std::vector<Bind*>());
  yy::stg_parser parser(p);
  int v = parser.parse();
  return v;
}
