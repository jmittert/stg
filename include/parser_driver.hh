#ifndef PARSER_DRIVER_HH
# define PARSER_DRIVER_HH
# include <string>
# include <map>
# include "ast.hh"
# include "parser.hh"

//class parser_driver;
// Tell Flex the lexer's prototype ...
# define YY_DECL int yylex (yy::stg_parser::semantic_type* yylval, yy::stg_parser::location_type* yylloc, parser_driver& driver)
// ... and declare it for the parser's sake.
YY_DECL;

class parser_driver {
public:
  parser_driver();
  virtual ~parser_driver();
  int result;

  Prog p = Prog(std::vector<Bind>());

  void scan_begin();
  void scan_end();
  bool trace_scanning;


  // Parse a file, return 0 on success
  int parse(const std::string& f);

  // name of the file to parse
  std::string file;

  bool trace_parsing;

  //error handling
  void error(const yy::location& l, const std::string&m);
  void error(const std::string&m);
};

#endif
