#ifndef BUILDER_EMITTER_H
#define BUILDER_EMITTER_H

#include "tree.h"
#include "tree_views.h"
#include "llvm/Support/raw_ostream.h"
#include <map>

struct MatMulInfo {
  std::string C;
  std::string A;
  std::string B;
};

struct ReshapeInfo {
  std::string lhs;
  std::string rhs;

  std::vector<std::string> lhsIndexes;
  std::vector<std::string> rhsIndexes;

  // where part.
  std::string newVar;
  std::vector<std::string> oldVars;
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
  bool matchAndEmitMatmul();
  bool matchMatMul(MatMulInfo &mmi);
  void emitMatMul(const MatMulInfo &mmi);

  // Reshape.
  bool matchAndEmitReshape();
  bool matchReshape(ReshapeInfo &rti);
  void emitReshape(const ReshapeInfo &rti);

  // Transpose
  // bool matchAndEmitTranspose();
  // bool matchTranspose(ReshapeAndTransposeInfo &rti);
  void emitTranspose(const TransposeInfo &rti);

private:
  lang::Comprehension comprehension_;
  llvm::raw_ostream &os;
  static thread_local SymbolTableMap symbolTable_;
};

#endif
