#include "dsl/emitter.h"
#include "dsl/parser.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TableGen/TableGenBackend.h"
#include "gtest/gtest.h"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace llvm;
using namespace lang;

void emitTactic(Parser &p, llvm::raw_ostream &os) {
  llvm::emitSourceFileHeader("Tactics", os);

  auto tac = Tac(p.parseTactic());
  auto stmts = tac.statements();
  os << "def " << Ident(tac.name()).name() << " : Tactics<";
  Emitter(stmts[0], os).emitWhat();
  os << "[\n";

  // what = how
  if (stmts.size() == 1)
    Emitter(stmts[0], os).emitHow();
  else {
    for (size_t i = 1; i < stmts.size(); i++) {
      Emitter(stmts[i], os).emitHow();
    }
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
                         "Outputs<[\"tmp0\"]>, StrExpr<\"{0,2,1}\">>,";
  std::string builder2 = "reshapeBuilder<Inputs<[\"tmp0\"]>, Outputs<[\"D\"]>, "
                         "StrExpr<\"{0,1}\">>,";
  std::string builder3 = "reshapeBuilder<Inputs<[\"A\"]>, Outputs<[\"E\"]>, "
                         "StrExpr<\"{0,1}\">>,";
  std::string builder4 =
      "matmulBuilder<StrExpr<\"N\">, StrExpr<\"N\">, M<1>, N<1>, "
      "K<1>, Constant<\"1\">, Constant<\"1\">, Inputs<[\"E\",\"B\"]>, "
      "Outputs<[\"D\"]>>,";
  std::string builder5 = "reshapeBuilder<Inputs<[\"D\"]>, Outputs<[\"tmp1\"]>, "
                         "StrExpr<{\"\"}>>,";
  std::string builder6 = "permutationBuilder<Inputs<[\"tmp1\"]>, "
                         "Outputs<[\"C\"]>, StrExpr<\"{0,2,1}\">>,";
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
                         "Outputs<[\"D\"]>, StrExpr<\"{0,2,1}\">>,";
  std::string builder2 = "reshapeBuilder<Inputs<[\"D\"]>, Outputs<[\"E\"]>, "
                         "StrExpr<\"{0,1}\">>,";
  std::string builder3 = "reshapeBuilder<Inputs<[\"A\"]>, Outputs<[\"F\"]>, "
                         "StrExpr<\"{0,1}\">>,";
  std::string builder4 =
      "matmulBuilder<StrExpr<\"N\">, StrExpr<\"N\">, M<1>, N<1>, "
      "K<1>, Constant<\"1\">, Constant<\"1\">, Inputs<[\"F\",\"B\"]>, "
      "Outputs<[\"E\"]>>,";
  std::string builder5 = "reshapeBuilder<Inputs<[\"E\"]>, Outputs<[\"tmp2\"]>, "
                         "StrExpr<{\"\"}>>,";
  std::string builder6 = "permutationBuilder<Inputs<[\"tmp2\"]>, "
                         "Outputs<[\"C\"]>, StrExpr<\"{0,2,1}\">>,";
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
    what = how 
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
      "matmulBuilder<StrExpr<\"N\">, StrExpr<\"N\">, M<1>, N<1>, "
      "K<1>, Constant<\"1\">, Constant<\"1\">, Inputs<[\"A\",\"B\"]>, "
      "Outputs<[\"C\"]>>,";

  auto patternPos = res.find(pattern);
  auto builder1Pos = res.find(builder1);

  ASSERT_TRUE(patternPos != std::string::npos);
  ASSERT_TRUE(builder1Pos != std::string::npos);
}

// Check transposition.
TEST(DslTest, shouldBeLoweredToAStrExprposeGemmCallForB) {

  std::string raw = R"(
  def GEMM {
    what = how
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
      "matmulBuilder<StrExpr<\"N\">, StrExpr<\"T\">, M<1>, N<1>, "
      "K<1>, Constant<\"1\">, Constant<\"1\">, Inputs<[\"A\",\"B\"]>, "
      "Outputs<[\"C\"]>>,";

  auto patternPos = res.find(pattern);
  auto builder1Pos = res.find(builder1);

  ASSERT_TRUE(patternPos != std::string::npos);
  ASSERT_TRUE(builder1Pos != std::string::npos);
}

// Check transposition.
TEST(DslTest, shouldBeLoweredToAStrExprposeGemmCallForA) {

  std::string raw = R"(
  def GEMM {
    what = how
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
      "matmulBuilder<StrExpr<\"T\">, StrExpr<\"N\">, M<1>, N<1>, "
      "K<1>, Constant<\"1\">, Constant<\"1\">, Inputs<[\"A\",\"B\"]>, "
      "Outputs<[\"C\"]>>,";

  auto patternPos = res.find(pattern);
  auto builder1Pos = res.find(builder1);

  ASSERT_TRUE(patternPos != std::string::npos);
  ASSERT_TRUE(builder1Pos != std::string::npos);
}

// Check transposition.
TEST(DslTest, shouldBeLoweredToAStrExprposeGemmCallForAandB) {

  std::string raw = R"(
  def GEMM {
    what = how
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
      "matmulBuilder<StrExpr<\"T\">, StrExpr<\"T\">, M<1>, N<1>, "
      "K<1>, Constant<\"1\">, Constant<\"1\">, Inputs<[\"A\",\"B\"]>, "
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
    what = how
    C(m, n) += A(m, k) * B(n, k)
  }
  )";
  Parser p = Parser(raw);

  std::string res;
  raw_string_ostream S{res};
  emitTactic(p, S);
  S.str();

  std::string pattern = "\"C(m, n) += A(m, k) * B(n, k)\"";
  std::string builder1 =
      "matmulBuilder<StrExpr<\"N\">, StrExpr<\"T\">, M<1>, N<1>, "
      "K<1>, Constant<\"1\">, Constant<\"1\">, Inputs<[\"A\",\"B\"]>, "
      "Outputs<[\"C\"]>>,";

  auto patternPos = res.find(pattern);
  auto builder1Pos = res.find(builder1);

  ASSERT_TRUE(patternPos != std::string::npos);
  ASSERT_TRUE(builder1Pos != std::string::npos);
}

// Execute the tensor using a *single*
// gemm call.
// Case 1.1 (see Tensor Contractions with Extended BLAS Kernels on CPU and GPU)
TEST(DslTest, shouldBeLoweredToASingleGemmCallCase1) {

  std::string raw = R"(
  def GEMM {
    what
    C(m, n, p) += A(m, k) * B(k, n, p) 
    how
    C(m, f) += A(m, k) * B(k, f) where f = n * p
  }
  )";
  Parser p = Parser(raw);

  std::string res;
  raw_string_ostream S{res};
  emitTactic(p, S);
  S.str();

  std::string pattern = "\"C(m, n, p) += A(m, k) * B(k, n, p)\"";
  std::string builder1 =
      "matmulBuilder<StrExpr<\"N\">, StrExpr<\"N\">, M<1>, N<2>, "
      "K<1>, Constant<\"1\">, Constant<\"1\">, Inputs<[\"A\",\"B\"]>, "
      "Outputs<[\"C\"]>>,";

  auto patternPos = res.find(pattern);
  auto builder1Pos = res.find(builder1);

  ASSERT_TRUE(patternPos != std::string::npos);
  ASSERT_TRUE(builder1Pos != std::string::npos);
}

// Case 1.5
TEST(DslTest, shouldBeLoweredToASingleGemmCallCase2) {

  std::string raw = R"(
  def GEMM {
    what 
    C(m, n, p) += A(m, k) * B(n, p, k)
    how
    C(m, f) += A(m, k) * B(f, k) where f = n * p
  }
  )";
  Parser p = Parser(raw);

  std::string res;
  raw_string_ostream S{res};
  emitTactic(p, S);
  S.str();

  std::string pattern = "\"C(m, n, p) += A(m, k) * B(n, p, k)\"";
  std::string builder1 =
      "matmulBuilder<StrExpr<\"N\">, StrExpr<\"T\">, M<1>, N<2>, "
      "K<1>, Constant<\"1\">, Constant<\"1\">, Inputs<[\"A\",\"B\"]>, "
      "Outputs<[\"C\"]>>,";

  auto patternPos = res.find(pattern);
  auto builder1Pos = res.find(builder1);

  ASSERT_TRUE(patternPos != std::string::npos);
  ASSERT_TRUE(builder1Pos != std::string::npos);
}

// Case 2.1
TEST(DslTest, shouldBeLoweredToASingleGemmCallCase3) {

  std::string raw = R"(
  def GEMM {
    what
    C(m, n, p) += A(k, m) * B(k, n, p)
    how
    C(m, f) += A(k, m) * B(k, f) where f = n * p
  }
  )";
  Parser p = Parser(raw);

  std::string res;
  raw_string_ostream S{res};
  emitTactic(p, S);
  S.str();

  std::string pattern = "\"C(m, n, p) += A(k, m) * B(k, n, p)\"";
  std::string builder1 =
      "matmulBuilder<StrExpr<\"T\">, StrExpr<\"N\">, M<1>, N<2>, "
      "K<1>, Constant<\"1\">, Constant<\"1\">, Inputs<[\"A\",\"B\"]>, "
      "Outputs<[\"C\"]>>,";

  auto patternPos = res.find(pattern);
  auto builder1Pos = res.find(builder1);

  ASSERT_TRUE(patternPos != std::string::npos);
  ASSERT_TRUE(builder1Pos != std::string::npos);
}

// Case 2.5
TEST(DslTest, shouldBeLoweredToASingleGemmCallCase4) {

  std::string raw = R"(
  def GEMM {
    what 
    C(m, n, p) += A(k, m) * B(n, p, k)
    how
    C(m, f) += A(k, m) * B(f, k) where f = n * p
  }
  )";
  Parser p = Parser(raw);

  std::string res;
  raw_string_ostream S{res};
  emitTactic(p, S);
  S.str();

  std::string pattern = "\"C(m, n, p) += A(k, m) * B(n, p, k)\"";
  std::string builder1 =
      "matmulBuilder<StrExpr<\"T\">, StrExpr<\"T\">, M<1>, N<2>, "
      "K<1>, Constant<\"1\">, Constant<\"1\">, Inputs<[\"A\",\"B\"]>, "
      "Outputs<[\"C\"]>>,";

  auto patternPos = res.find(pattern);
  auto builder1Pos = res.find(builder1);

  ASSERT_TRUE(patternPos != std::string::npos);
  ASSERT_TRUE(builder1Pos != std::string::npos);
}

// Case 5.1
TEST(DslTest, shouldBeLoweredToASingleGemmCallCase5) {

  std::string raw = R"(
  def GEMM {
    what 
    C(m, n, p) += B(k, m, n) * A(p, k)
    how
    C(f, p) += B(k, f) * A(p, k) where f = m * n
  }
  )";
  Parser p = Parser(raw);

  std::string res;
  raw_string_ostream S{res};
  emitTactic(p, S);
  S.str();

  std::string pattern = "\"C(m, n, p) += B(k, m, n) * A(p, k)\"";
  std::string builder1 =
      "matmulBuilder<StrExpr<\"T\">, StrExpr<\"T\">, M<2>, N<1>, "
      "K<1>, Constant<\"1\">, Constant<\"1\">, Inputs<[\"B\",\"A\"]>, "
      "Outputs<[\"C\"]>>,";

  auto patternPos = res.find(pattern);
  auto builder1Pos = res.find(builder1);

  ASSERT_TRUE(patternPos != std::string::npos);
  ASSERT_TRUE(builder1Pos != std::string::npos);
}

// Case 5.5
TEST(DslTest, shouldBeLoweredToASingleGemmCallCase6) {

  std::string raw = R"(
  def GEMM {
    what
    C(m, n, p) += B(m, n, k) * A(p, k)
    how
    C(f, p) += B(f, k) * A(p, k) where f = m * n
  }
  )";
  Parser p = Parser(raw);

  std::string res;
  raw_string_ostream S{res};
  emitTactic(p, S);
  S.str();

  std::string pattern = "\"C(m, n, p) += B(m, n, k) * A(p, k)\"";
  std::string builder1 =
      "matmulBuilder<StrExpr<\"N\">, StrExpr<\"T\">, M<2>, N<1>, "
      "K<1>, Constant<\"1\">, Constant<\"1\">, Inputs<[\"B\",\"A\"]>, "
      "Outputs<[\"C\"]>>,";

  auto patternPos = res.find(pattern);
  auto builder1Pos = res.find(builder1);

  ASSERT_TRUE(patternPos != std::string::npos);
  ASSERT_TRUE(builder1Pos != std::string::npos);
}

// Case 6.1
TEST(DslTest, shouldBeLoweredToASingleGemmCallCase7) {

  std::string raw = R"(
  def GEMM {
    what 
    C(m, n, p) += B(k, m, n) * A(k, p)
    how
    C(f, p) += B(k, f) * A(k, p) where f = m * n
  }
  )";
  Parser p = Parser(raw);

  std::string res;
  raw_string_ostream S{res};
  emitTactic(p, S);
  S.str();

  std::string pattern = "\"C(m, n, p) += B(k, m, n) * A(k, p)\"";
  std::string builder1 =
      "matmulBuilder<StrExpr<\"T\">, StrExpr<\"N\">, M<2>, N<1>, "
      "K<1>, Constant<\"1\">, Constant<\"1\">, Inputs<[\"B\",\"A\"]>, "
      "Outputs<[\"C\"]>>,";

  auto patternPos = res.find(pattern);
  auto builder1Pos = res.find(builder1);

  ASSERT_TRUE(patternPos != std::string::npos);
  ASSERT_TRUE(builder1Pos != std::string::npos);
}

// Case 6.5
TEST(DslTest, shouldBeLoweredToASingleGemmCallCase8) {

  std::string raw = R"(
  def GEMM {
    what 
    C(m, n, p) += B(m, n, k) * A(k, p)
    how
    C(f, p) += B(f, k) * A(k, p) where f = m * n
  }
  )";
  Parser p = Parser(raw);

  std::string res;
  raw_string_ostream S{res};
  emitTactic(p, S);
  S.str();

  std::string pattern = "\"C(m, n, p) += B(m, n, k) * A(k, p)\"";
  std::string builder1 =
      "matmulBuilder<StrExpr<\"N\">, StrExpr<\"N\">, M<2>, N<1>, "
      "K<1>, Constant<\"1\">, Constant<\"1\">, Inputs<[\"B\",\"A\"]>, "
      "Outputs<[\"C\"]>>,";

  auto patternPos = res.find(pattern);
  auto builder1Pos = res.find(builder1);

  ASSERT_TRUE(patternPos != std::string::npos);
  ASSERT_TRUE(builder1Pos != std::string::npos);
}

// This is the equivalent of the previous test
// but a naive version where we add reshape operations.
// We may want to have some rules that allow capturing
// such inefficiencies.
TEST(DslTest, shouldBeLoweredToAGemmCallAndMultipleReshapes) {

  std::string raw = R"(
  def GEMM {
    what
    C(m, n, p) += A(m, k) * B(k, n, p)
    how
    D(m, f) = C(m, n, p) where f = n * p
    E(k, f) = B(k, n, p) where f = n * p
    D(m, f) += A(m, k) * E(k, f)
    C(m, n, p) = D(m, f) where f = n * p
  }
  )";
  Parser p = Parser(raw);

  std::string res;
  raw_string_ostream S{res};
  emitTactic(p, S);
  S.str();

  std::string pattern = "\"C(m, n, p) += A(m, k) * B(k, n, p)\"";
  std::string builder1 = "reshapeBuilder<Inputs<[\"C\"]>, Outputs<[\"D\"]>, "
                         "StrExpr<\"{1,2}\">>,";
  std::string builder2 = "reshapeBuilder<Inputs<[\"B\"]>, Outputs<[\"E\"]>, "
                         "StrExpr<\"{1,2}\">>,";
  std::string builder3 =
      "matmulBuilder<StrExpr<\"N\">, StrExpr<\"N\">, M<1>, N<1>, "
      "K<1>, Constant<\"1\">, Constant<\"1\">, Inputs<[\"A\",\"E\"]>, "
      "Outputs<[\"D\"]>>,";
  std::string builder4 = "reshapeBuilder<Inputs<[\"D\"]>, Outputs<[\"C\"]>, "
                         "StrExpr<{\"\"}>>,";
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

TEST(DslTest, shouldBeLoweredToAGemmCallAndMultipleStrExprposesAndReshapes) {

	std::string raw = R"(
		def TENSOR {
			what
			C(a, b) += A(c, a, d) * B(d, c, b)
			how
			D(a, c, d) = A(c, a, d)
			E(c, d, b) = B(d, c, b)
			F(a, f) = D(a, c, d) where f = c * d
			H(f, b) = E(c, d, b) where f = c * d
			C(a, b) += F(a, f) * H(f, b)
		}
	)";
	Parser p = Parser(raw);
	
	std::string res;
	raw_string_ostream S{res};
	emitTactic(p, S);
	S.str();

	std::string pattern = "\"C(a, b) += A(c, a, d) * B(d, c, b)\"";
	
	auto patternPos = res.find(pattern);

	ASSERT_TRUE(patternPos != std::string::npos);
}

TEST(DslTest, shouldLowerToGemm) {

  std::string raw = R"(
  def GEMM {
    what = how
    C(i, j) += A(i, k) * B(k, j)
  }
  )";
  Parser p = Parser(raw);

  std::string res;
  raw_string_ostream S{res};
  emitTactic(p, S);
  S.str();

  std::string pattern = "\"C(i, j) += A(i, k) * B(k, j)\"";
  std::string builder1 =
      "matmulBuilder<StrExpr<\"N\">, StrExpr<\"N\">, M<1>, N<1>, "
      "K<1>, Constant<\"1\">, Constant<\"1\">, Inputs<[\"A\",\"B\"]>, "
      "Outputs<[\"C\"]>>,";

  auto patternPos = res.find(pattern);
  auto builder1Pos = res.find(builder1);

  ASSERT_TRUE(patternPos != std::string::npos);
  ASSERT_TRUE(builder1Pos != std::string::npos);

  if(const char* env = std::getenv("DUMP_TEST")) {
    if (atoi(env) == 1)
      std::cout << S.str() << "\n";
  }

}

TEST(DslTest, shouldLowerToMatVec) {

	std::string raw = R"(
		def GEMV {
			what = how
			x(i) += A(i, j) * y(j)
		}
	)";
	Parser p = Parser(raw);
	
	std::string res;
	raw_string_ostream S{res};
	emitTactic(p, S);
	S.str();

	std::string pattern = "\"x(i) += A(i, j) * y(j)\"";
	std::string builder1 =
			"matvecBuilder<StrExpr<\"N\">, Inputs<[\"A\",\"y\"]>, Outputs<[\"x\"]>, " 
			"Constant<\"1\">, Constant<\"1\">>,";
	
	auto patternPos = res.find(pattern);
	auto builder1Pos = res.find(builder1);

	ASSERT_TRUE(patternPos != std::string::npos);
	ASSERT_TRUE(builder1Pos != std::string::npos);
}

TEST(DslTest, shouldLowerToMatVecTrans) {

	std::string raw = R"(
		def GEMV {
			what = how
			x(i) += A(j, i) * y(j)
		}
	)";
	Parser p = Parser(raw);
	
	std::string res;
	raw_string_ostream S{res};
	emitTactic(p, S);
	S.str();

	std::string pattern = "\"x(i) += A(j, i) * y(j)\"";
	std::string builder1 =
			"matvecBuilder<StrExpr<\"T\">, Inputs<[\"A\",\"y\"]>, Outputs<[\"x\"]>, " 
			"Constant<\"1\">, Constant<\"1\">>,";
	
	auto patternPos = res.find(pattern);
	auto builder1Pos = res.find(builder1);

	ASSERT_TRUE(patternPos != std::string::npos);
	ASSERT_TRUE(builder1Pos != std::string::npos);
}

TEST(DslTest, shouldLowerToMatVecWithConstantAlpha) {

	std::string raw = R"(
		def GEMV {
			what = how
			x(i) += alpha * A(i, j) * y(j)
		}
	)";
	Parser p = Parser(raw);
	
	std::string res;
	raw_string_ostream S{res};
	emitTactic(p, S);
	std::cout << S.str() << std::endl;
}
