#include "dsl/parser.h"
#include <iostream>
#include <string>

int main() {
  using namespace lang;
  std::string raw = R"(
  def TTGT {
    what
    C(a,b,c) += A(a,c,d) * B(d,b)
    how
    C(f,b) = C(a,b,c) where f = a * c
    A(f,d) = A(a,c,d)
    C(f,b) += A(f,d) * B(d,b) 
    C(a,b,c) = C(f,b) where f = a * c
  }
  )";
  std::cout << raw << std::endl;

  Parser p = Parser(raw);
  auto tac = Tac(p.parseTactic());
  std::cout << Ident(tac.name()).name() << std::endl;
  auto stmts = tac.statements();

  for (size_t i = 0; i < stmts.size(); i++) {
    auto comprehension = Comprehension(stmts[i]);
    std::cout << comprehension << std::endl;
  }

  return 0;
}
