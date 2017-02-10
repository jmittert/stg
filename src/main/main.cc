#include <iostream>
#include "ast.hh"
#include "parser.hh"
#include "eval.hh"
#include "parser_driver.hh"

using namespace std;
int main(int argc, char** argv) {
  if (argc != 2) {
    cerr << "Usage: " << argv[0] << " file name" << endl;
    return 1;
  }
  parser_driver driver;
  for (int i = 1; i < argc; ++i) {
    if (argv[i] == std::string("-p"))
      driver.trace_parsing = true;
    else if (argv[i] == std::string("-s"))
      driver.trace_scanning = true;
    else if (!driver.parse(argv[i])) {
      Prog* p = driver.p;
      cout << *p << endl;
      int res = eval(*p);
      cout << "Result: " << res << endl;
    } else
      return 1;
  }
}
