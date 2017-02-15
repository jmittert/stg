#include <ast.hh>

std::string vecToString(const std::vector<Bind>& binds) {
  std::stringstream ss;
  ss << " ";
  for (auto& bind: binds) {
    ss << bind << " ";
  }
  return ss.str();
}

std::string vecToString(const std::vector<Var>& vars) {
  std::stringstream ss;
  ss << " ";
  for (auto& var: vars) {
    ss << var << " ";
  }
  return ss.str();
}

std::string vecToString(const std::vector<std::unique_ptr<Expr>>& v) {
  std::stringstream ss;
  ss << " ";
  for (auto& e: v) {
    ss << *e << " ";
  }
  return ss.str();
}

std::string vecToString(const std::vector<std::unique_ptr<Atom>>& v) {
  std::stringstream ss;
  ss << " ";
  for (auto& e: v) {
    ss << *e << " ";
  }
  return ss.str();
}

std::string vecToString(const std::vector<AAlt>& vars) {
  std::stringstream ss;
  ss << " ";
  for (auto& var: vars) {
    ss << var << " ";
  }
  return ss.str();
}

std::string vecToString(const std::vector<PAlt>& vars) {
  std::stringstream ss;
  ss << " ";
  for (auto& var: vars) {
    ss << var << " ";
  }
  return ss.str();
}
