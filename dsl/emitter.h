#ifndef BUILDER_EMITTER_H
#define BUILDER_EMITTER_H

#include "tree.h"
#include "tree_views.h"
#include "llvm/Support/raw_ostream.h"
#include <map>

enum class Trans { N, T };

// TODO add better support for conv.
struct ConvInfo {
  std::string out;
  std::string filt;
  std::string img;
};

struct MatMulInfo {
  std::string C;
  std::string A;
  std::string B;

  std::string m;
  std::string n;
  std::string k;

  Trans transa;
  Trans transb;

  std::string alpha;
  std::string beta;

  int dimensionsForM;
  int dimensionsForN;
  int dimensionsForK;
};

struct MatVecInfo {
  std::string x;
  std::string A;
  std::string y;

  Trans transa;

  std::string alpha;
  std::string beta;
};

struct ReshapeInfo {
  std::string lhs;
  std::string rhs;

  std::vector<std::string> lhsIndexes;
  std::vector<std::string> rhsIndexes;

  // where part.
  std::vector<std::string> newVar;
  std::vector<std::vector<std::string>> oldVars;
};

struct TransposeInfo {
  std::string lhs;
  std::string rhs;

  std::vector<size_t> permutation;
};

struct Tensor {
  std::string name_;
  std::vector<std::string> indices_;
};

class SymbolTableMap {
public:
  SymbolTableMap() : nextId_(0), lastEmittedVar_(""){};
  std::string getNextVariable();
  std::string getLastEmittedVariable() const;

private:
  size_t nextId_;
  std::string lastEmittedVar_;
  // key tensor name, value Tensor.
  std::map<std::string, Tensor> symbolTable_;
};

class Emitter {
public:
  Emitter(lang::Comprehension co, llvm::raw_ostream &os)
      : comprehension_(co), os(os) {}
  void emitHow();
  void emitWhat();

  // MatMul.
  bool matchAndEmitMatMul();
  bool matchMatMul(MatMulInfo &mmi);
  void emitMatMul(const MatMulInfo &mmi);

  // MatVec.
  bool matchAndEmitMatVec();
  bool matchMatVec(MatVecInfo &mvi);
  void emitMatVec(const MatVecInfo &mvi);

  // Reshape.
  bool matchAndEmitReshape();
  bool matchReshape(ReshapeInfo &rti);
  void emitReshape(const ReshapeInfo &rti);
  // TODO: remove me and handle multiple where clauses
  // better.
  void printGroup(const ReshapeInfo &rti);

  // Transpose
  bool matchAndEmitTranspose();
  bool matchTranspose(TransposeInfo &rti);
  void emitTranspose(const TransposeInfo &rti);

  // Conv
  bool matchAndEmitConv();
  bool matchConv(ConvInfo &cvi);
  void emitConv(const ConvInfo &cvi);

private:
  lang::Comprehension comprehension_;
  llvm::raw_ostream &os;
  static thread_local SymbolTableMap symbolTable_;
};

#endif
