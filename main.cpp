#include "dsl/emitter.h"
#include "dsl/parser.h"
#include <fstream>
#include <string>

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TableGen/TableGenBackend.h"

using namespace lang;

void emitTactic(ListView<Comprehension> stmts, llvm::raw_ostream &os) {
  llvm::emitSourceFileHeader("Tactics", os);
  if (stmts.size() < 2)
    return;
  Emitter(stmts[0], os).emitWhat();
  for (size_t i = 1; i < stmts.size(); i++) {
    Emitter(stmts[i], os).emitHow();
  }
  os << "eraseOpBuilder \n";
  os << "]>;";
}

int main() {

  std::string raw = R"(
  def TTGT {
    what
    C(a,b,c) += A(a,c,d) * B(d,b)
    how
    D(f,b) = C(a,b,c) where f = a * c 
    E(f,d) = A(a,c,d) where f = a * c
    D(f,b) += E(f,d) * B(d,b)
    C(a,b,c) = D(f,b) where f = a * c
  } 
  )";

  Parser p = Parser(raw);
  auto tac = Tac(p.parseTactic());
  emitTactic(tac.statements(), llvm::outs());

  return 0;
}
