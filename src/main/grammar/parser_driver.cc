#include "parser_driver.hh"
#include "parser.hh"

parser_driver::parser_driver ()
  : trace_scanning (false), trace_parsing (false)
{
}

parser_driver::~parser_driver ()
{
}

int
parser_driver::parse (const std::string &f)
{
  file = f;
  scan_begin ();
  yy::stg_parser parser (*this);
  parser.set_debug_level (trace_parsing);
  int res = parser.parse ();
  scan_end ();
  return res;
}

void
parser_driver::error (const yy::location& l, const std::string& m)
{
  std::cerr << l << ": " << m << std::endl;
}

void
parser_driver::error (const std::string& m)
{
  std::cerr << m << std::endl;
}
