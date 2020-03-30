#include "dsl/emitter.h"
#include "dsl/parser.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TableGen/TableGenBackend.h"
#include "gtest/gtest.h"
#include <iostream>
#include <string>

using namespace llvm;
using namespace lang;

void emitTactic(Parser &p, llvm::raw_ostream &os) {
  llvm::emitSourceFileHeader("Tactics", os);

  auto tac = Tac(p.parseTactic());
  auto stmts = tac.statements();
  os << "def " << Ident(tac.name()).name() << " : Tactics<";
  Emitter(stmts[0], os).emitWhat();
  os << ", [\n";

  for (size_t i = 1; i < stmts.size(); i++) {
    Emitter(stmts[i], os).emitHow();
  }
  os.indent(2) << "eraseOpBuilder\n";
  os << "]>;\n";
}

TEST(DslTest, tensor) {

  std::string raw = R"(
  def TTGT {
    what
    C(a,b,c) += A(a,c,d) * B(d,b) + K(o)
    how
    D(f,b) = C(a,b,c) where f = a * c 
    E(f,d) = A(a,c,d) where f = a * c
    D(f,b) += E(f,d) * B(d,b)
    C(a,b,c) = D(f,b) where f = a * c
  } 
  )";
  Parser p = Parser(raw);

  std::string res;
  raw_string_ostream S{res};
  emitTactic(p, S);
  S.str();
  std::string pattern = "\"C(a, b, c) += A(a, c, d) * B(d, b) + K(o)\"";
  std::string builder1 = "permutationBuilder<Inputs<[\"C\"]>, "
                         "Outputs<[\"tmp0\"]>, AffineExpression<\"{0,2,1}\">>,";
  std::string builder2 = "reshapeBuilder<Inputs<[\"tmp0\"]>, Outputs<[\"D\"]>, "
                         "AffineExpression<\"{0,1}\">>,";
  std::string builder3 = "reshapeBuilder<Inputs<[\"A\"]>, Outputs<[\"E\"]>, "
                         "AffineExpression<\"{0,1}\">>,";
  std::string builder4 =
      "matmulBuilder<Inputs<[\"E\",\"B\"]>, Outputs<[\"D\"]>>,";
  std::string builder5 = "reshapeBuilder<Inputs<[\"D\"]>, Outputs<[\"tmp1\"]>, "
                         "AffineExpression<{\"\"}>>,";
  std::string builder6 = "permutationBuilder<Inputs<[\"tmp1\"]>, "
                         "Outputs<[\"C\"]>, AffineExpression<\"{0,2,1}\">>,";
  std::string builder7 = "eraseOpBuilder";

  auto patternPos = res.find(pattern);
  auto builder1Pos = res.find(builder1);
  auto builder2Pos = res.find(builder2);
  auto builder3Pos = res.find(builder3);
  auto builder4Pos = res.find(builder4);
  auto builder5Pos = res.find(builder5);
  auto builder6Pos = res.find(builder6);

  ASSERT_TRUE(patternPos != std::string::npos);
  ASSERT_TRUE(builder1Pos != std::string::npos);
  ASSERT_TRUE(builder2Pos != std::string::npos);
  ASSERT_TRUE(builder3Pos != std::string::npos);
  ASSERT_TRUE(builder4Pos != std::string::npos);
  ASSERT_TRUE(builder5Pos != std::string::npos);
  ASSERT_TRUE(builder6Pos != std::string::npos);
}
