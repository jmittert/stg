#include <iostream>
#include "ast.hh"
#include "parser.hh"
#include "eval.hh"

using namespace std;
int main() {
  Prog p = Prog(new std::vector<Bind*>());
  yy::stg_parser parser(p);
  int v = parser.parse();
  if (v == 0) {
    cout << p << endl;
    int res = eval(p);
    cout << "Result: " << res << endl;
  }
  return v;
}
