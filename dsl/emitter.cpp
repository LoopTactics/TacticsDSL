#include "emitter.h"
#include "matchers.h"
#include <iostream>

using namespace lang;

thread_local SymbolTableMap Emitter::symbolTable_;

std::string SymbolTableMap::getNextVariable() {
  std::string res = "tmp" + std::to_string(nextId_++);
  lastEmittedVar_ = res;
  return res;
}

std::string SymbolTableMap::getLastEmittedVariable() const {
  return lastEmittedVar_;
}

// Resursively maps the function `fn` to `tree` and all of its
// descendants in preorder - teckyl style.
void applyRecursive(const TreeRef &tree,
                    std::function<void(const TreeRef &)> fn) {
  fn(tree);
  for (auto e : tree->trees())
    applyRecursive(e, fn);
}

bool isTN(Comprehension c, MatMulInfo &mmi) {
  using namespace matchers;
  auto ctx = m_ctx();
  auto _A = m_ArrayPlaceholder();
  auto _B = m_ArrayPlaceholder();
  auto _i = m_Placeholder();
  auto _j = m_Placeholder();
  auto _k = m_Placeholder();
  auto a = m_Access(_A({_k, _i}));
  auto b = m_Access(_B({_k, _j}));
  auto matcher = m_Mul(a, b);
  if (!matcher.match(c.rhs()))
    return false;
  auto C = c.ident().name();
  if ((C == ctx[_A]) || (C == ctx[_B]))
    return false;
  auto indexC = c.indices();
  if ((indexC[0].name() != ctx[_i]) || (indexC[1].name() != ctx[_j]))
    return false;
  mmi.A = ctx[_A];
  mmi.B = ctx[_B];
  mmi.C = C;
  mmi.m = ctx[_i];
  mmi.n = ctx[_j];
  mmi.k = ctx[_k];
  return true;
}

bool isNN(Comprehension c, MatMulInfo &mmi) {
  using namespace matchers;
  auto ctx = m_ctx();
  auto _A = m_ArrayPlaceholder();
  auto _B = m_ArrayPlaceholder();
  auto _i = m_Placeholder();
  auto _j = m_Placeholder();
  auto _k = m_Placeholder();
  auto a = m_Access(_A({_i, _k}));
  auto b = m_Access(_B({_k, _j}));
  auto matcher = m_Mul(a, b);
  if (!matcher.match(c.rhs()))
    return false;
  auto C = c.ident().name();
  if ((C == ctx[_A]) || (C == ctx[_B]))
    return false;
  auto indexC = c.indices();
  if ((indexC[0].name() != ctx[_i]) || (indexC[1].name() != ctx[_j]))
    return false;
  mmi.A = ctx[_A];
  mmi.B = ctx[_B];
  mmi.C = C;
  mmi.m = ctx[_i];
  mmi.n = ctx[_j];
  mmi.k = ctx[_k];
  return true;
}

bool isNT(Comprehension c, MatMulInfo &mmi) {
  using namespace matchers;
  auto ctx = m_ctx();
  auto _A = m_ArrayPlaceholder();
  auto _B = m_ArrayPlaceholder();
  auto _i = m_Placeholder();
  auto _j = m_Placeholder();
  auto _k = m_Placeholder();
  auto a = m_Access(_A({_i, _k}));
  auto b = m_Access(_B({_j, _k}));
  auto matcher = m_Mul(a, b);
  if (!matcher.match(c.rhs()))
    return false;
  auto C = c.ident().name();
  if ((C == ctx[_A]) || (C == ctx[_B]))
    return false;
  auto indexC = c.indices();
  if ((indexC[0].name() != ctx[_i]) || (indexC[1].name() != ctx[_j]))
    return false;
  mmi.A = ctx[_A];
  mmi.B = ctx[_B];
  mmi.C = C;
  mmi.m = ctx[_i];
  mmi.n = ctx[_j];
  mmi.k = ctx[_k];
  return true;
}

bool isTT(Comprehension c, MatMulInfo &mmi) {
  using namespace matchers;
  auto ctx = m_ctx();
  auto _A = m_ArrayPlaceholder();
  auto _B = m_ArrayPlaceholder();
  auto _i = m_Placeholder();
  auto _j = m_Placeholder();
  auto _k = m_Placeholder();
  auto a = m_Access(_A({_k, _i}));
  auto b = m_Access(_B({_j, _k}));
  auto matcher = m_Mul(a, b);
  if (!matcher.match(c.rhs()))
    return false;
  auto C = c.ident().name();
  if ((C == ctx[_A]) || (C == ctx[_B]))
    return false;
  auto indexC = c.indices();
  if ((indexC[0].name() != ctx[_i]) || (indexC[1].name() != ctx[_j]))
    return false;
  mmi.A = ctx[_A];
  mmi.B = ctx[_B];
  mmi.C = C;
  mmi.m = ctx[_i];
  mmi.n = ctx[_j];
  mmi.k = ctx[_k];
  return true;
}

// check C[i][j] += A[i][k] * B[k][j]
bool Emitter::matchMatMul(MatMulInfo &mmi) {
  if (comprehension_.assignment()->kind() != TK_PLUS_EQ)
    return false;

  if (comprehension_.indices().size() != 2)
    return false;

  bool matchedFlag = false;
  bool aTransFlag = false;
  bool bTransFlag = false;
  if (isNN(comprehension_, mmi)) {
    matchedFlag = true;
  }
  if (isTN(comprehension_, mmi)) {
    matchedFlag = true;
    aTransFlag = true;
  }
  if (isNT(comprehension_, mmi)) {
    matchedFlag = true;
    bTransFlag = true;
  }
  if (isTT(comprehension_, mmi)) {
    matchedFlag = true;
    aTransFlag = true;
    bTransFlag = true;
  }
  if (!matchedFlag)
    return false;

  // check if there is a where clause.
  auto where = comprehension_.whereClauses();
  if (where.size() > 1)
    throw ErrorReport(comprehension_) << "expect single where clause.";

  int letSize = 1;
  std::string letVar = "nullptr";
  if (where.size() == 1) {
    auto let = Let(where[0]);
    letVar = let.name().name();
    applyRecursive(let.rhs(), [&](const TreeRef &t) {
      if (t->kind() == TK_IDENT)
        letSize++;
    });
  }

  // fill mmi.
  mmi.transa = (aTransFlag) ? Trans::T : Trans::N;
  mmi.transb = (bTransFlag) ? Trans::T : Trans::N;
  mmi.alpha = 1;
  mmi.beta = 1;
  mmi.dimensionsForM = (letVar == mmi.m) ? letSize - 1 : 1;
  mmi.dimensionsForN = (letVar == mmi.n) ? letSize - 1 : 1;
  mmi.dimensionsForK = (letVar == mmi.k) ? letSize - 1 : 1;
  return true;
}

std::string toString(Trans t) {
  switch (t) {
  case Trans::N:
    return "N";
  case Trans::T:
    return "T";
  }
}

void Emitter::emitMatMul(const MatMulInfo &mmi) {
  os.indent(2) << "matmulBuilder<"
               << "Trans<\"" << toString(mmi.transa) << "\">, "
               << "Trans<\"" << toString(mmi.transb) << "\">, "
               << "Dims<" << mmi.dimensionsForM << ">, "
               << "Dims<" << mmi.dimensionsForN << ">, "
               << "Dims<" << mmi.dimensionsForK << ">, "
               << "Constant<" << mmi.alpha << ">, "
               << "Constant<" << mmi.beta << ">, "
               << "Inputs<["
               << "\"" << mmi.A << "\""
               << ","
               << "\"" << mmi.B << "\""
               << "]>, Outputs<["
               << "\"" << mmi.C << "\""
               << "]>>,\n";
}

bool Emitter::matchAndEmitMatmul() {
  MatMulInfo mmi;
  if (matchMatMul(mmi)) {
    emitMatMul(mmi);
    return true;
  }
  return false;
}

// check A[...] = X[...]
bool Emitter::matchReshape(ReshapeInfo &ri) {
  if (comprehension_.assignment()->kind() != '=')
    return false;
  auto rhs = comprehension_.rhs();
  int rhsOperands = 0;
  applyRecursive(rhs, [&](const TreeRef &t) {
    if (t->kind() == TK_APPLY)
      rhsOperands++;
  });
  if (rhsOperands != 1)
    return false;
  auto lhsIndexes = comprehension_.indices();
  auto rhsIndexes = Apply(rhs).arguments();
  if (lhsIndexes.size() == rhsIndexes.size())
    return false;
  // all the reshape operation must have one where clause.
  auto where = comprehension_.whereClauses();
  if (where.size() != 1)
    return false;

  // fill ri.
  ri.lhs = comprehension_.ident().name();
  ri.rhs = Apply(comprehension_.rhs()).name().name();

  for (const auto &elem : lhsIndexes) {
    ri.lhsIndexes.push_back(elem.name());
  }
  for (const auto &elem : rhsIndexes) {
    ri.rhsIndexes.push_back(Ident(elem).name());
  }

  auto let = Let(where[0]);
  ri.newVar = let.name().name();
  applyRecursive(let.rhs(), [&](const TreeRef &t) {
    if (t->kind() == TK_IDENT)
      ri.oldVars.push_back(Ident(t).name());
  });
  if (ri.oldVars.size() < 2)
    throw ErrorReport(let) << "rhs of where expect one or more variable";
  return true;
}

// helper.
bool find(const std::string &target,
          const std::vector<std::string> &arrayToInspect) {
  auto it = std::find(arrayToInspect.begin(), arrayToInspect.end(), target);
  if (it == arrayToInspect.end())
    return false;
  return true;
}

// helper.
bool find(const std::vector<std::string> &targets,
          const std::vector<std::string> &arrayToInspect) {
  for (const auto &elem : targets)
    if (!find(elem, arrayToInspect))
      return false;
  return true;
}

// helper. Find "what" in "target", substitute "what" with "with" and remove
// "what" from "target".
void substitute(std::vector<std::string> &target, const std::string what,
                const std::vector<std::string> with) {
  auto it = std::find(target.begin(), target.end(), what);
  if (it == target.end())
    return;
  target.insert(it, with.begin(), with.end());
  target.erase(std::remove(target.begin(), target.end(), what), target.end());
}

// return the ordering of dimension between source and dest.
std::vector<size_t> getOrdering(const std::vector<std::string> &source,
                                const std::vector<std::string> &dest) {
  assert(source.size() == dest.size() && "expect same size");
  std::vector<size_t> ordering;
  for (size_t i = 0; i < dest.size(); i++)
    for (size_t j = 0; j < source.size(); j++)
      if (dest[i] == source[j])
        ordering.push_back(j);
  return ordering;
}

bool isConsecutive(std::vector<size_t> ordering) {
  bool isTrue = true;
  for (size_t i = 1; i < ordering.size(); i++) {
    if (ordering[i] != ordering[i - 1] + 1) {
      isTrue = false;
      break;
    }
  }
  return isTrue;
}

std::vector<std::string> reorder(const std::vector<std::string> &indexes,
                                 const std::vector<size_t> &ordering) {
  assert(indexes.size() == ordering.size() && "expect same size");
  std::vector<std::string> res;
  for (size_t i = 0; i < indexes.size(); i++)
    res.push_back(indexes[ordering[i]]);
  return res;
}

void Emitter::emitReshape(const ReshapeInfo &ri) {
  // Assuming where f = a * c
  // check if the reshape dimension (f) is on the LHS or RHS.
  bool isOnLhs = find(ri.newVar, ri.lhsIndexes);
  bool isOnRhs = find(ri.newVar, ri.rhsIndexes);
  if (!(isOnRhs ^ isOnLhs))
    throw ErrorReport(comprehension_)
        << "You want to reshape " << ri.newVar
        << "but it is not on the LHS nor on the RHS.";

  // check if a and c are on the RHS if f is on the LHS and viceversa.
  if (isOnRhs && !find(ri.oldVars, ri.lhsIndexes))
    throw ErrorReport(comprehension_)
        << "The dimensions you want to rehsape are not in the expected side";
  if (isOnLhs && !find(ri.oldVars, ri.rhsIndexes))
    throw ErrorReport(comprehension_)
        << "The dimensions you want to rehsape are not in the expected side";

  auto lhsIndexesCpy = ri.lhsIndexes;
  auto rhsIndexesCpy = ri.rhsIndexes;

  // substitute f.
  if (isOnLhs)
    substitute(lhsIndexesCpy, ri.newVar, ri.oldVars);
  else
    substitute(rhsIndexesCpy, ri.newVar, ri.oldVars);

  std::vector<size_t> indexes;
  for (auto elem : ri.oldVars) {
    auto it = std::find(lhsIndexesCpy.begin(), lhsIndexesCpy.end(), elem);
    assert(it != lhsIndexesCpy.end());
    indexes.push_back(std::distance(lhsIndexesCpy.begin(), it));
  }

  // check if we need to transpose.
  bool requireTranspose = false;
  auto ordering = getOrdering(lhsIndexesCpy, rhsIndexesCpy);
  if (!isConsecutive(ordering))
    requireTranspose = true;

  // if f is rhs emit first reshape and then transpose
  // if necessary.
  if (isOnRhs) {
    std::string dest =
        (requireTranspose) ? symbolTable_.getNextVariable() : ri.lhs;
    os.indent(2) << "reshapeBuilder<Inputs<["
                 << "\"" << ri.rhs << "\""
                 << "]>, Outputs<["
                 << "\"" << dest << "\""
                 << "]>, AffineExpression<{\"\"}>>,\n";
  }

  bool emittedTranspose = false;
  if (requireTranspose) {
    if (isOnRhs)
      emitTranspose({ri.lhs, symbolTable_.getLastEmittedVariable(), ordering});
    else
      emitTranspose({symbolTable_.getNextVariable(), ri.rhs, ordering});
    emittedTranspose = true;
  }

  if (isOnLhs) {
    auto newLhs =
        (emittedTranspose) ? symbolTable_.getLastEmittedVariable() : ri.lhs;
    os.indent(2) << "reshapeBuilder<Inputs<[";
    if (!emittedTranspose) {
      os << "\"" << ri.rhs << "\""
         << "]>, Outputs<["
         << "\"" << newLhs << "\"";
    } else {
      os << "\"" << newLhs << "\""
         << "]>, Outputs<["
         << "\"" << ri.lhs << "\"";
    }
    os << "]>, AffineExpression<\"{";
    for (size_t i = 0; i < indexes.size(); i++) {
      if (i == indexes.size() - 1)
        os << indexes[i];
      else
        os << indexes[i] << ",";
    }
    os << "}\">>,\n";
  }
}

bool Emitter::matchAndEmitReshape() {
  ReshapeInfo ri;
  if (matchReshape(ri)) {
    emitReshape(ri);
    return true;
  }
  return false;
}

void Emitter::emitTranspose(const TransposeInfo &ti) {
  os.indent(2) << "permutationBuilder<Inputs<["
               << "\"" << ti.rhs << "\""
               << "]>, Outputs<["
               << "\"" << ti.lhs << "\""
               << "]>, AffineExpression<"
               << "\"{";
  for (size_t i = 0; i < ti.permutation.size(); i++) {
    if (i == ti.permutation.size() - 1)
      os << ti.permutation[i];
    else
      os << ti.permutation[i] << ",";
  }
  os << "}\">>,\n";
}

bool Emitter::matchTranspose(TransposeInfo &rti) {
  if (comprehension_.assignment()->kind() != '=')
    return false;
  auto rhs = comprehension_.rhs();
  int rhsOperands = 0;
  applyRecursive(rhs, [&](const TreeRef &t) {
    if (t->kind() == TK_APPLY)
      rhsOperands++;
  });
  if (rhsOperands != 1)
    return false;
  auto lhsIndexes = comprehension_.indices();
  auto rhsIndexes = Apply(rhs).arguments();
  if (lhsIndexes.size() != rhsIndexes.size())
    return false;
  // not where clause allowed.
  auto where = comprehension_.whereClauses();
  if (where.size() != 0)
    return false;
  std::vector<std::string> lhsIndexesAsStr, rhsIndexesAsStr;
  for (const auto &elem : lhsIndexes)
    lhsIndexesAsStr.push_back(elem.name());
  for (const auto &elem : rhsIndexes)
    rhsIndexesAsStr.push_back(Ident(elem).name());
  if (!find(lhsIndexesAsStr, rhsIndexesAsStr))
    return false;
  // fill rti.
  rti.lhs = comprehension_.ident().name();
  rti.rhs = Apply(rhs).name().name();
  rti.permutation = getOrdering(rhsIndexesAsStr, lhsIndexesAsStr);
  return true;
}

bool Emitter::matchAndEmitTranspose() {
  TransposeInfo rti;
  if (matchTranspose(rti)) {
    emitTranspose(rti);
    return true;
  }
  return false;
}

void Emitter::emitHow() {
  if (matchAndEmitMatmul())
    return;
  if (matchAndEmitReshape())
    return;
  if (matchAndEmitTranspose())
    return;
  throw ErrorReport(comprehension_) << "unknown builder";
}

static void recursivelyEmitRhs(const TreeRef &t, llvm::raw_ostream &os) {
  switch (t->kind()) {
  case '+': {
    recursivelyEmitRhs(t->trees().at(0), os);
    os << " + ";
    recursivelyEmitRhs(t->trees().at(1), os);
    return;
  }
  case '*': {
    recursivelyEmitRhs(t->trees().at(0), os);
    os << " * ";
    recursivelyEmitRhs(t->trees().at(1), os);
    return;
  }
  case TK_APPLY: {
    auto name = Apply(t).name().name();
    os << name << "(";
    auto indices = Apply(t).arguments();
    for (size_t i = 0; i < indices.size(); i++) {
      if (i == indices.size() - 1)
        os << Ident(indices[i]).name();
      else
        os << Ident(indices[i]).name() << ", ";
    }
    os << ")";
    return;
  }
  }
  throw ErrorReport(t) << "expect only TK_APPLY, '+' and '*'";
}

void Emitter::emitWhat() {
  auto lhs = comprehension_.ident().name();
  auto lhsIndexes = comprehension_.indices();
  auto assignment = comprehension_.assignment();

  os << "\"" << lhs << "(";
  for (size_t i = 0; i < lhsIndexes.size(); i++) {
    if (i == lhsIndexes.size() - 1)
      os << lhsIndexes[i].name();
    else
      os << lhsIndexes[i].name() << ", ";
  }
  os << ")";
  switch (assignment->kind()) {
  case '=':
    os << " = ";
    break;
  case TK_PLUS_EQ:
    os << " += ";
    break;
  default:
    throw ErrorReport(assignment) << "assignment not available yet";
  }

  auto rhs = comprehension_.rhs();
  recursivelyEmitRhs(rhs, os);
  os << "\"";
}
