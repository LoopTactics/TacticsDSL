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

TEST(DslTest, tensorWithCompactBuilder) {

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

  std::string res;
  raw_string_ostream S{res};
  emitTactic(p, S);
  S.str();

  std::string pattern = "\"C(a, b, c) += A(a, c, d) * B(d, b)\"";
  std::string builder1 = "permutationBuilder<Inputs<[\"C\"]>, "
                         "Outputs<[\"tmp0\"]>, AffineExpression<\"{0,2,1}\">>,";
  std::string builder2 = "reshapeBuilder<Inputs<[\"tmp0\"]>, Outputs<[\"D\"]>, "
                         "AffineExpression<\"{0,1}\">>,";
  std::string builder3 = "reshapeBuilder<Inputs<[\"A\"]>, Outputs<[\"E\"]>, "
                         "AffineExpression<\"{0,1}\">>,";
  std::string builder4 =
      "matmulBuilder<Trans<[\"N\"]>, Trans<[\"N\"]>, Dims<[1]>, Dims<[1]>, "
      "Dims<[1]>, Constant<1>, Constant<1>, Inputs<[\"E\",\"B\"]>, "
      "Outputs<[\"D\"]>>,";
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

TEST(DslTest, tensorWithFullBuilderSpecification) {

  std::string raw = R"(
  def TTGT {
    what 
    C(a, b, c) += A(a, c, d) * B(d, b)
    how 
    D(a, c, b) = C(a, b, c)
    E(f, b) = D(a, c, b) where f = a * c
    F(f, d) = A(a, c, d) where f = a * c
    E(f, b) += F(f, d) * B(d, b)
    C(a, b, c) = E(f, b) where f = a * c
  }
  )";
  Parser p = Parser(raw);

  std::string res;
  raw_string_ostream S{res};
  emitTactic(p, S);
  S.str();

  std::string pattern = "\"C(a, b, c) += A(a, c, d) * B(d, b)\"";
  std::string builder1 = "permutationBuilder<Inputs<[\"C\"]>, "
                         "Outputs<[\"D\"]>, AffineExpression<\"{0,2,1}\">>,";
  std::string builder2 = "reshapeBuilder<Inputs<[\"D\"]>, Outputs<[\"E\"]>, "
                         "AffineExpression<\"{0,1}\">>,";
  std::string builder3 = "reshapeBuilder<Inputs<[\"A\"]>, Outputs<[\"F\"]>, "
                         "AffineExpression<\"{0,1}\">>,";
  std::string builder4 =
      "matmulBuilder<Trans<[\"N\"]>, Trans<[\"N\"]>, Dims<[1]>, Dims<[1]>, "
      "Dims<[1]>, Constant<1>, Constant<1>, Inputs<[\"F\",\"B\"]>, "
      "Outputs<[\"E\"]>>,";
  std::string builder5 = "reshapeBuilder<Inputs<[\"E\"]>, Outputs<[\"tmp2\"]>, "
                         "AffineExpression<{\"\"}>>,";
  std::string builder6 = "permutationBuilder<Inputs<[\"tmp2\"]>, "
                         "Outputs<[\"C\"]>, AffineExpression<\"{0,2,1}\">>,";
  std::string builder7 = "eraseOpBuilder";

  auto patternPos = res.find(pattern);
  auto builder1Pos = res.find(builder1);
  auto builder2Pos = res.find(builder2);
  auto builder3Pos = res.find(builder3);
  auto builder4Pos = res.find(builder4);
  auto builder5Pos = res.find(builder5);
  auto builder6Pos = res.find(builder6);
  auto builder7Pos = res.find(builder7);

  ASSERT_TRUE(patternPos != std::string::npos);
  ASSERT_TRUE(builder1Pos != std::string::npos);
  ASSERT_TRUE(builder2Pos != std::string::npos);
  ASSERT_TRUE(builder3Pos != std::string::npos);
  ASSERT_TRUE(builder4Pos != std::string::npos);
  ASSERT_TRUE(builder5Pos != std::string::npos);
  ASSERT_TRUE(builder6Pos != std::string::npos);
  ASSERT_TRUE(builder7Pos != std::string::npos);
}

// Check Gemm
TEST(DslTest, checkGemmCall) {

  std::string raw = R"(
  def GEMM {
    what 
    C(m, n) += A(m, k) * B(k, n)
    how
    C(m, n) += A(m, k) * B(k, n)
  }
  )";
  Parser p = Parser(raw);

  std::string res;
  raw_string_ostream S{res};
  emitTactic(p, S);
  S.str();

  std::string pattern = "\"C(m, n) += A(m, k) * B(k, n)\"";
  std::string builder1 =
      "matmulBuilder<Trans<[\"N\"]>, Trans<[\"N\"]>, Dims<[1]>, Dims<[1]>, "
      "Dims<[1]>, Constant<1>, Constant<1>, Inputs<[\"A\",\"B\"]>, "
      "Outputs<[\"C\"]>>,";

  auto patternPos = res.find(pattern);
  auto builder1Pos = res.find(builder1);

  ASSERT_TRUE(patternPos != std::string::npos);
  ASSERT_TRUE(builder1Pos != std::string::npos);
}

// Check transposition.
TEST(DslTest, shouldBeLoweredToATransposeGemmCallForB) {

  std::string raw = R"(
  def GEMM {
    what
    C(i, j) += A(i, k) * B(j, k)
    how
    C(i, j) += A(i, k) * B(j, k)
  }
  )";
  Parser p = Parser(raw);

  std::string res;
  raw_string_ostream S{res};
  emitTactic(p, S);
  S.str();

  std::string pattern = "\"C(i, j) += A(i, k) * B(j, k)\"";
  std::string builder1 =
      "matmulBuilder<Trans<[\"N\"]>, Trans<[\"T\"]>, Dims<[1]>, Dims<[1]>, "
      "Dims<[1]>, Constant<1>, Constant<1>, Inputs<[\"A\",\"B\"]>, "
      "Outputs<[\"C\"]>>,";

  auto patternPos = res.find(pattern);
  auto builder1Pos = res.find(builder1);

  ASSERT_TRUE(patternPos != std::string::npos);
  ASSERT_TRUE(builder1Pos != std::string::npos);
}

// Check transposition.
TEST(DslTest, shouldBeLoweredToATransposeGemmCallForA) {

  std::string raw = R"(
  def GEMM {
    what
    C(i, j) += A(k, i) * B(k, j)
    how 
    C(i, j) += A(k, i) * B(k, j)
  }
  )";
  Parser p = Parser(raw);

  std::string res;
  raw_string_ostream S{res};
  emitTactic(p, S);
  S.str();

  std::string pattern = "\"C(i, j) += A(k, i) * B(k, j)\"";
  std::string builder1 =
      "matmulBuilder<Trans<[\"T\"]>, Trans<[\"N\"]>, Dims<[1]>, Dims<[1]>, "
      "Dims<[1]>, Constant<1>, Constant<1>, Inputs<[\"A\",\"B\"]>, "
      "Outputs<[\"C\"]>>,";

  auto patternPos = res.find(pattern);
  auto builder1Pos = res.find(builder1);

  ASSERT_TRUE(patternPos != std::string::npos);
  ASSERT_TRUE(builder1Pos != std::string::npos);
}

// Check transposition.
TEST(DslTest, shouldBeLoweredToATransposeGemmCallForAandB) {

  std::string raw = R"(
  def GEMM {
    what
    C(i, j) += A(k, i) * B(j, k)
    how 
    C(i, j) += A(k, i) * B(j, k)
  }
  )";
  Parser p = Parser(raw);

  std::string res;
  raw_string_ostream S{res};
  emitTactic(p, S);
  S.str();

  std::string pattern = "\"C(i, j) += A(k, i) * B(j, k)\"";
  std::string builder1 =
      "matmulBuilder<Trans<[\"T\"]>, Trans<[\"T\"]>, Dims<[1]>, Dims<[1]>, "
      "Dims<[1]>, Constant<1>, Constant<1>, Inputs<[\"A\",\"B\"]>, "
      "Outputs<[\"C\"]>>,";

  auto patternPos = res.find(pattern);
  auto builder1Pos = res.find(builder1);

  ASSERT_TRUE(patternPos != std::string::npos);
  ASSERT_TRUE(builder1Pos != std::string::npos);
}

// Check beta
TEST(DslTest, shouldHaveBetaSetToZero) {

  std::string raw = R"(
  def GEMM {
    what
    C(m, n) = A(m, k) * B(n, k)
    how
    C(m, n) = A(m, k) * B(n, k)
  }
  )";
  Parser p = Parser(raw);

  std::string res;
  raw_string_ostream S{res};
  emitTactic(p, S);
  S.str();

  std::string pattern = "\"C(m, n) = A(m, k) * B(n, k)\"";
  std::string builder1 =
      "matmulBuilder<Trans<[\"N\"]>, Trans<[\"T\"]>, Dims<[1]>, Dims<[1]>, "
      "Dims<[1]>, Constant<1>, Constant<0>, Inputs<[\"A\",\"B\"]>, "
      "Outputs<[\"C\"]>>,";

  auto patternPos = res.find(pattern);
  auto builder1Pos = res.find(builder1);

  ASSERT_TRUE(patternPos != std::string::npos);
  ASSERT_TRUE(builder1Pos != std::string::npos);
}

// Execute the tensor using a *single*
// gemm call.
TEST(DslTest, shouldBeLoweredToAGemmCall) {

  std::string raw = R"(
  def GEMM {
    what
    C(m, n, p) = A(m, k) * B(k, n, p) 
    how
    C(m, f) = A(m, k) * B(k, f) where f = n * p
  }
  )";
}

// This is the equivalent of the previous test
// but a naive version where we add reshape operations.
// We may want to have some rules that allow capturing
// such inefficiencies.
TEST(DslTest, shouldBeLoweredToAGemmCallAndMultipleReshapes) {

  std::string raw = R"(
  def GEMM {
    what
    C(m, n, p) = A(m, k) * B(k, n, p)
    how
    D(m, f) = C(m, n, p) where f = n * p
    E(k, f) = B(k, n, p) where f = n * p
    D(m, f) = A(m, k) * E(k, f)
    C(m, n, p) = D(m, f) where f = n * p
  }
  )";
  Parser p = Parser(raw);

  std::string res;
  raw_string_ostream S{res};
  emitTactic(p, S);
  S.str();

  std::string pattern = "\"C(m, n, p) = A(m, k) * B(k, n, p)\"";
  std::string builder1 = "reshapeBuilder<Inputs<[\"C\"]>, Outputs<[\"D\"]>, "
                         "AffineExpression<\"{1,2}\">>,";
  std::string builder2 = "reshapeBuilder<Inputs<[\"B\"]>, Outputs<[\"E\"]>, "
                         "AffineExpression<\"{1,2}\">>,";
  std::string builder3 =
      "matmulBuilder<Trans<[\"N\"]>, Trans<[\"N\"]>, Dims<[1]>, Dims<[1]>, "
      "Dims<[1]>, Constant<1>, Constant<0>, Inputs<[\"A\",\"E\"]>, "
      "Outputs<[\"D\"]>>,";
  std::string builder4 = "reshapeBuilder<Inputs<[\"D\"]>, Outputs<[\"C\"]>, "
                         "AffineExpression<{\"\"}>>,";
  std::string builder5 = "eraseOpBuilder";

  auto patternPos = res.find(pattern);
  auto builder1Pos = res.find(builder1);
  auto builder2Pos = res.find(builder2);
  auto builder3Pos = res.find(builder3);
  auto builder4Pos = res.find(builder4);
  auto builder5Pos = res.find(builder5);

  ASSERT_TRUE(patternPos != std::string::npos);
  ASSERT_TRUE(builder1Pos != std::string::npos);
  ASSERT_TRUE(builder2Pos != std::string::npos);
  ASSERT_TRUE(builder3Pos != std::string::npos);
  ASSERT_TRUE(builder4Pos != std::string::npos);
  ASSERT_TRUE(builder5Pos != std::string::npos);
}
