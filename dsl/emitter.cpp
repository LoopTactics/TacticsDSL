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
static void applyRecursive(const TreeRef &tree,
                           std::function<void(const TreeRef &)> fn) {
  fn(tree);
  for (auto e : tree->trees())
    applyRecursive(e, fn);
}

static bool isTN(Comprehension c, MatMulInfo &mmi) {
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
  if (indexC.size() != 2)
    return false;
  if ((indexC[0].name() != ctx[_i]) || (indexC[1].name() != ctx[_j]))
    return false;
  mmi.A = ctx[_A];
  mmi.B = ctx[_B];
  mmi.C = C;
  mmi.m = ctx[_i];
  mmi.n = ctx[_j];
  mmi.k = ctx[_k];
  mmi.alpha = "1";
  mmi.beta = "1";
  return true;
}

static bool isNN(Comprehension c, MatMulInfo &mmi) {
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
  if (indexC.size() != 2)
    return false;
  if ((indexC[0].name() != ctx[_i]) || (indexC[1].name() != ctx[_j]))
    return false;
  mmi.A = ctx[_A];
  mmi.B = ctx[_B];
  mmi.C = C;
  mmi.m = ctx[_i];
  mmi.n = ctx[_j];
  mmi.k = ctx[_k];
  mmi.alpha = "1";
  mmi.beta = "1";
  return true;
}

static bool isNNWithAlpha(Comprehension c, MatMulInfo &mmi) {
  using namespace matchers;
  auto ctx = m_ctx();
  auto _A = m_ArrayPlaceholder();
  auto _B = m_ArrayPlaceholder();
  auto _i = m_Placeholder();
  auto _j = m_Placeholder();
  auto _k = m_Placeholder();
  auto a = m_Access(_A({_i, _k}));
  auto b = m_Access(_B({_k, _j}));
  auto matcher = m_Mul(m_Any(), m_Mul(a, b));
  if (!matcher.match(c.rhs()))
    return false;
  auto C = c.ident().name();
  if ((C == ctx[_A]) || (C == ctx[_B]))
    return false;
  auto indexC = c.indices();
  if (indexC.size() != 2)
    return false;
  if ((indexC[0].name() != ctx[_i]) || (indexC[1].name() != ctx[_j]))
    return false;
  mmi.A = ctx[_A];
  mmi.B = ctx[_B];
  mmi.C = C;
  mmi.m = ctx[_i];
  mmi.n = ctx[_j];
  mmi.k = ctx[_k];
  mmi.alpha = "alpha";
  mmi.beta = "1";
  return true;
}

static bool isNT(Comprehension c, MatMulInfo &mmi) {
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
  if (indexC.size() != 2)
    return false;
  if ((indexC[0].name() != ctx[_i]) || (indexC[1].name() != ctx[_j]))
    return false;
  mmi.A = ctx[_A];
  mmi.B = ctx[_B];
  mmi.C = C;
  mmi.m = ctx[_i];
  mmi.n = ctx[_j];
  mmi.k = ctx[_k];
  mmi.alpha = "1";
  mmi.beta = "1";
  return true;
}

static bool isTT(Comprehension c, MatMulInfo &mmi) {
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
  if (indexC.size() != 2)
    return false;
  if ((indexC[0].name() != ctx[_i]) || (indexC[1].name() != ctx[_j]))
    return false;
  mmi.A = ctx[_A];
  mmi.B = ctx[_B];
  mmi.C = C;
  mmi.m = ctx[_i];
  mmi.n = ctx[_j];
  mmi.k = ctx[_k];
  mmi.alpha = "1";
  mmi.beta = "1";
  return true;
}

static bool isN(Comprehension c, MatVecInfo &mvi) {
  using namespace matchers;
  auto ctx = m_ctx();
  auto _y = m_ArrayPlaceholder();
  auto _A = m_ArrayPlaceholder();
  auto _i = m_Placeholder();
  auto _j = m_Placeholder();
  auto a = m_Access(_A({_i, _j}));
  auto b = m_Access(_y({_j}));
  auto matcher = m_Mul(a, b);
  if (!matcher.match(c.rhs()))
    return false;
  auto x = c.ident().name();
  if ((x == ctx[_A]) || (x == ctx[_y]))
    return false;
  auto indexX = c.indices();
  if (indexX.size() != 1)
    return false;
  if (indexX[0].name() != ctx[_i])
    return false;
  mvi.A = ctx[_A];
  mvi.y = ctx[_y];
  mvi.x = x;
  mvi.transa = Trans::N;
  mvi.alpha = "1";
  mvi.beta = "1";
  return true;
}

static bool isNWithConstantAlpha(Comprehension c, MatVecInfo &mvi) {
  using namespace matchers;
  auto ctx = m_ctx();
  auto _y = m_ArrayPlaceholder();
  auto _A = m_ArrayPlaceholder();
  auto _i = m_Placeholder();
  auto _j = m_Placeholder();
  auto a = m_Access(_A({_i, _j}));
  auto b = m_Access(_y({_j}));
  auto matcherWithConst = m_Mul(m_Any(), m_Mul(a, b));
  if (!matcherWithConst.match(c.rhs()))
    return false;
  auto x = c.ident().name();
  if ((x == ctx[_A]) || (x == ctx[_y]))
    return false;
  auto indexX = c.indices();
  if (indexX.size() != 1)
    return false;
  if (indexX[0].name() != ctx[_i])
    return false;
  mvi.A = ctx[_A];
  mvi.y = ctx[_y];
  mvi.x = x;
  mvi.transa = Trans::N;
  mvi.alpha = "alpha";
  mvi.beta = "1";
  return true;
}

static bool isTWithConstantAlpha(Comprehension c, MatVecInfo &mvi) {
  using namespace matchers;
  auto ctx = m_ctx();
  auto _y = m_ArrayPlaceholder();
  auto _A = m_ArrayPlaceholder();
  auto _i = m_Placeholder();
  auto _j = m_Placeholder();
  auto a = m_Access(_A({_j, _i}));
  auto b = m_Access(_y({_j}));
  auto matcherWithConst = m_Mul(m_Any(), m_Mul(a, b));
  if (!matcherWithConst.match(c.rhs()))
    return false;
  auto x = c.ident().name();
  if ((x == ctx[_A]) || (x == ctx[_y]))
    return false;
  auto indexX = c.indices();
  if (indexX.size() != 1)
    return false;
  if (indexX[0].name() != ctx[_i])
    return false;
  mvi.A = ctx[_A];
  mvi.y = ctx[_y];
  mvi.x = x;
  mvi.transa = Trans::T;
  mvi.alpha = "alpha";
  mvi.beta = "1";
  return true;
}

static bool isT(Comprehension c, MatVecInfo &mvi) {
  using namespace matchers;
  auto ctx = m_ctx();
  auto _y = m_ArrayPlaceholder();
  auto _A = m_ArrayPlaceholder();
  auto _i = m_Placeholder();
  auto _j = m_Placeholder();
  auto a = m_Access(_A({_j, _i}));
  auto b = m_Access(_y({_j}));
  auto matcher = m_Mul(a, b);
  if (!matcher.match(c.rhs()))
    return false;
  auto x = c.ident().name();
  if ((x == ctx[_A]) || (x == ctx[_y]))
    return false;
  auto indexX = c.indices();
  if (indexX.size() != 1)
    return false;
  if (indexX[0].name() != ctx[_i])
    return false;
  mvi.A = ctx[_A];
  mvi.y = ctx[_y];
  mvi.x = x;
  mvi.transa = Trans::T;
  mvi.alpha = "1";
  mvi.beta = "1";
  return true;
}

// check if we are dealing with matmul.
bool Emitter::matchMatMul(MatMulInfo &mmi) {
  if (comprehension_.assignment()->kind() != TK_PLUS_EQ)
    return false;

  if (comprehension_.indices().size() != 2)
    return false;

  bool matchedFlag = false;
  bool aTransFlag = false;
  bool bTransFlag = false;
  if (isNN(comprehension_, mmi) || isNNWithAlpha(comprehension_, mmi)) {
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
  mmi.dimensionsForM = (letVar == mmi.m) ? letSize - 1 : 1;
  mmi.dimensionsForN = (letVar == mmi.n) ? letSize - 1 : 1;
  mmi.dimensionsForK = (letVar == mmi.k) ? letSize - 1 : 1;
  return true;
}

// check if we are dealing with a matvec.
bool Emitter::matchMatVec(MatVecInfo &mvi) {
  if (comprehension_.assignment()->kind() != TK_PLUS_EQ)
    return false;

  if (comprehension_.indices().size() != 1)
    return false;

  // TODO: merge isN with isNWithTranspose once we
  // introduce a method to clear the context.
  return (isN(comprehension_, mvi) || isT(comprehension_, mvi) ||
          isNWithConstantAlpha(comprehension_, mvi) ||
          isTWithConstantAlpha(comprehension_, mvi));
}

std::string toString(Trans t) {
  switch (t) {
  case Trans::N:
    return "N";
  case Trans::T:
    return "T";
  }
  assert(0 && "invalid case");
  return "null";
}

void Emitter::emitMatMul(const MatMulInfo &mmi) {
  os.indent(2) << "matmulBuilder<"
               << "StrExpr<\"" << toString(mmi.transa) << "\">, "
               << "StrExpr<\"" << toString(mmi.transb) << "\">, "
               << "M<" << mmi.dimensionsForM << ">, "
               << "N<" << mmi.dimensionsForN << ">, "
               << "K<" << mmi.dimensionsForK << ">, "
               << "Constant<\"" << mmi.alpha << "\">, "
               << "Constant<\"" << mmi.beta << "\">, "
               << "Inputs<["
               << "\"" << mmi.A << "\""
               << ","
               << "\"" << mmi.B << "\""
               << "]>, Outputs<["
               << "\"" << mmi.C << "\""
               << "]>>,\n";
}

void Emitter::emitMatVec(const MatVecInfo &mvi) {
  os.indent(2) << "matvecBuilder<"
               << "StrExpr<\"" << toString(mvi.transa) << "\">, "
               << "Inputs<["
               << "\"" << mvi.A << "\""
               << ","
               << "\"" << mvi.y << "\""
               << "]>, Outputs<["
               << "\"" << mvi.x << "\""
               << "]>, "
               << "Constant<\"" << mvi.alpha << "\">, "
               << "Constant<\"" << mvi.beta << "\">>, \n";
}

bool Emitter::matchAndEmitMatMul() {
  MatMulInfo mmi;
  if (matchMatMul(mmi)) {
    emitMatMul(mmi);
    return true;
  }
  return false;
}

bool Emitter::matchAndEmitMatVec() {
  MatVecInfo mvi;
  if (matchMatVec(mvi)) {
    emitMatVec(mvi);
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
  // if (where.size() != 1)
  //  return false;

  // fill ri.
  ri.lhs = comprehension_.ident().name();
  ri.rhs = Apply(comprehension_.rhs()).name().name();

  for (const auto &elem : lhsIndexes) {
    ri.lhsIndexes.push_back(elem.name());
  }
  for (const auto &elem : rhsIndexes) {
    ri.rhsIndexes.push_back(Ident(elem).name());
  }

  for (size_t i = 0; i < where.size(); i++) {
    auto let = Let(where[i]);
    ri.newVar.push_back(let.name().name());
    std::vector<std::string> tmp;
    applyRecursive(let.rhs(), [&tmp](const TreeRef &t) {
      if (t->kind() == TK_IDENT)
        tmp.push_back(Ident(t).name());
    });
    if (tmp.size() < 2)
      throw ErrorReport(let) << "rhs of where expect one or more variable";
    ri.oldVars.push_back(tmp);
  }
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

std::string getReshapeMap(const std::vector<size_t> &toReshape,
                          const std::vector<size_t> &notToReshape) {
  assert(toReshape.size());
  assert(notToReshape.size());
  std::string res = "\"{";
  auto itReshape = std::find(toReshape.begin(), toReshape.end(), 0);
  auto itNotReshape = std::find(notToReshape.begin(), notToReshape.end(), 0);
  if ((itReshape == toReshape.end()) && (itNotReshape == notToReshape.end()))
    assert(0 && "cannot find zero dimension");

  if (itReshape != toReshape.end()) {
    res += "{";
    for (size_t i = 0; i < toReshape.size(); i++) {
      if (i == toReshape.size() - 1) {
        res += ", ";
        res += std::to_string(toReshape[i]);
      } else
        res += std::to_string(toReshape[i]);
    }
    if (notToReshape.size() == 1)
      res += "}";
    else
      res += "}, ";
    for (size_t i = 0; i < notToReshape.size(); i++) {
      if (i == notToReshape.size() - 1) {
        res += ", ";
        res += std::to_string(notToReshape[i]);
      } else
        res += std::to_string(notToReshape[i]);
    }
  }

  else {
    for (size_t i = 0; i < notToReshape.size(); i++) {
      if (i == notToReshape.size() - 1 && notToReshape.size() != 1) {
        res += ", ";
        res += std::to_string(notToReshape[i]);
      } else
        res += std::to_string(notToReshape[i]);
    }
    if (notToReshape.size() == 1)
      res += ", {";
    for (size_t i = 0; i < toReshape.size(); i++) {
      if (i == toReshape.size() - 1) {
        res += ", ";
        res += std::to_string(toReshape[i]);
      } else
        res += std::to_string(toReshape[i]);
    }
    res += "}";
  }

  res += "}\"";
  return res;
}

void getReshapeGroupImpl(std::vector<std::string> oldVars,
                         std::vector<std::string> indexes,
                         std::vector<size_t> &group) {
  for (auto var : oldVars) {
    auto pos = std::find(indexes.begin(), indexes.end(), var);
    group.push_back(std::distance(indexes.begin(), pos));
  }
}

std::vector<std::vector<size_t>>
getReshapeGroup(std::vector<std::string> newVar,
                std::vector<std::vector<std::string>> oldVars,
                std::vector<std::string> lhsIndexes,
                std::vector<std::string> rhsIndexes) {
  std::vector<std::vector<size_t>> res;
  for (size_t i = 0; i < newVar.size(); i++) {
    bool isOnLhs = find(newVar[i], lhsIndexes);
    bool isOnRhs = find(newVar[i], rhsIndexes);
    if (!(isOnRhs ^ isOnLhs)) {
      assert(0 && "cannot be on lhs or rhs");
    }

    std::vector<size_t> group;
    if (isOnLhs)
      getReshapeGroupImpl(oldVars[i], rhsIndexes, group);
    else
      getReshapeGroupImpl(oldVars[i], lhsIndexes, group);
    res.push_back(group);
  }
  return res;
}

std::string composeGroup(std::vector<std::vector<size_t>> groups) {
  assert(groups.size() == 2 && "max group size == 2");
  std::string res = "{";
  for (auto group : groups) {
    res += "{";
    for (size_t i = 0; i < group.size(); i++) {
      if (i == group.size() - 1)
        res += std::to_string(group[i]);
      else {
        res += std::to_string(group[i]);
        res += ", ";
      }
    }
    res += "}";
  }
  res += "}";
  return res;
}

void Emitter::printGroup(const ReshapeInfo &ri) {
  auto groups =
      getReshapeGroup(ri.newVar, ri.oldVars, ri.lhsIndexes, ri.rhsIndexes);
  os.indent(2) << "reshapeBuilder<Inputs<[";
  os << "\"" << ri.rhs << "\""
     << "]>, Outputs<["
     << "\"" << ri.lhs << "\"";
  os << "]>, StrExpr<";
  os << composeGroup(groups);
  os << ">>,\n";
}

void Emitter::emitReshape(const ReshapeInfo &ri) {
  assert(ri.newVar.size() == ri.oldVars.size());
  // horrible way of handling multiple wheres
  if (ri.newVar.size() == 2)
    return printGroup(ri);

  assert(ri.newVar.size() == 1);
  // Assuming where f = a * c
  // check if the reshape dimension (f) is on the LHS or RHS.
  bool isOnLhs = find(ri.newVar[0], ri.lhsIndexes);
  bool isOnRhs = find(ri.newVar[0], ri.rhsIndexes);
  if (!(isOnRhs ^ isOnLhs))
    throw ErrorReport(comprehension_)
        << "You want to reshape " << ri.newVar[0]
        << "but it is not on the LHS nor on the RHS.";

  // check if a and c are on the RHS if f is on the LHS and viceversa.
  if (isOnRhs && !find(ri.oldVars[0], ri.lhsIndexes))
    throw ErrorReport(comprehension_)
        << "The dimensions you want to rehsape are not in the expected side";
  if (isOnLhs && !find(ri.oldVars[0], ri.rhsIndexes))
    throw ErrorReport(comprehension_)
        << "The dimensions you want to rehsape are not in the expected side";

  auto lhsIndexesCpy = ri.lhsIndexes;
  auto rhsIndexesCpy = ri.rhsIndexes;

  // substitute f.
  if (isOnLhs)
    substitute(lhsIndexesCpy, ri.newVar[0], ri.oldVars[0]);
  else
    substitute(rhsIndexesCpy, ri.newVar[0], ri.oldVars[0]);

  std::vector<size_t> indexesToReshape;
  std::vector<size_t> indexesNotToReshape;
  for (auto elem : ri.oldVars[0]) {
    auto it = std::find(lhsIndexesCpy.begin(), lhsIndexesCpy.end(), elem);
    assert(it != lhsIndexesCpy.end());
    indexesToReshape.push_back(std::distance(lhsIndexesCpy.begin(), it));
  }
  assert(lhsIndexesCpy.size() == rhsIndexesCpy.size());
  for (size_t i = 0; i < lhsIndexesCpy.size(); i++) {
    auto it = std::find(indexesToReshape.begin(), indexesToReshape.end(), i);
    if (it == indexesToReshape.end())
      indexesNotToReshape.push_back(i);
  }

  // check if we need to transpose.
  bool requireTranspose = false;
  auto ordering = getOrdering(lhsIndexesCpy, rhsIndexesCpy);
  if (!isConsecutive(ordering))
    requireTranspose = true;

  // if f is rhs emit first reshape and then transpose
  // if necessary.
  if (isOnRhs) {
    auto it =
        std::find(ri.rhsIndexes.begin(), ri.rhsIndexes.end(), ri.newVar[0]);
    auto dist = std::distance(ri.rhsIndexes.begin(), it);
    indexesToReshape.clear();
    indexesNotToReshape.clear();
    for (size_t i = 0; i < ri.rhsIndexes.size(); i++)
      indexesToReshape.push_back(dist++);
    for (size_t i = 0; i < rhsIndexesCpy.size(); i++) {
      auto it = std::find(indexesToReshape.begin(), indexesToReshape.end(), i);
      if (it == indexesToReshape.end())
        indexesNotToReshape.push_back(i);
    }
    std::string dest =
        (requireTranspose) ? symbolTable_.getNextVariable() : ri.lhs;
    os.indent(2) << "reshapeBuilder<Inputs<["
                 << "\"" << ri.rhs << "\""
                 << "]>, Outputs<["
                 << "\"" << dest << "\""
                 << "]>, StrExpr<"
                 << getReshapeMap(indexesToReshape, indexesNotToReshape)
                 << ">>,\n";
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
    os << "]>, StrExpr<";
    os << getReshapeMap(indexesToReshape, indexesNotToReshape);
    os << ">>,\n";
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
  os.indent(2) << "transposeBuilder<Inputs<["
               << "\"" << ti.rhs << "\""
               << "]>, Outputs<["
               << "\"" << ti.lhs << "\""
               << "]>, StrExpr<"
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

// TODO: better handling for conv.
bool Emitter::matchConv(ConvInfo &cvi) {
  if (comprehension_.assignment()->kind() != TK_PLUS_EQ)
    return false;

  if (comprehension_.indices().size() != 2)
    return false;

  cvi.out = comprehension_.ident().name();
  cvi.filt = Apply(comprehension_.rhs()->trees().at(0)).name().name();
  cvi.img = Apply(comprehension_.rhs()->trees().at(1)).name().name();
  return true;
}

void Emitter::emitConv(const ConvInfo &cvi) {
  os.indent(2) << "convBuilder<"
               << "Inputs<["
               << "\"" << cvi.filt << "\""
               << ", "
               << "\"" << cvi.img << "\""
               << "]>, Outputs<["
               << "\"" << cvi.out << "\""
               << "]>, StrExpr<\"{1, 1, 1, 1}\">, StrExpr<\"{0, 0}\">>,\n";
  return;
}

bool Emitter::matchAndEmitConv() {
  ConvInfo cvi;
  if (matchConv(cvi)) {
    emitConv(cvi);
    return true;
  }
  return false;
}

void Emitter::emitHow() {

  if (matchAndEmitMatMul())
    return;
  if (matchAndEmitMatVec())
    return;

  if (matchAndEmitReshape())
    return;
  if (matchAndEmitTranspose())
    return;

  if (matchAndEmitConv())
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
      if (indices[i]->kind() != TK_IDENT) {
        if (i == indices.size() - 1)
          recursivelyEmitRhs(indices[i], os);
        else {
          recursivelyEmitRhs(indices[i], os);
          os << ", ";
        }
      } else {
        if (i == indices.size() - 1)
          os << Ident(indices[i]).name();
        else
          os << Ident(indices[i]).name() << ", ";
      }
    }
    os << ")";
    return;
  }
  case TK_IDENT: {
    os << Ident(t).name();
    return;
  }
  }
  throw ErrorReport(t) << "expect only TK_APPLY, TK_IDENT, '+' and '*' but "
                          "got"
                       << t->kind() << "\n";
}

void Emitter::emitWhat() {
  os << "def Tactic : Tactics<";
  if (comprehension_.whereClauses().size())
    throw ErrorReport(comprehension_)
        << "what part cannot have 'where' clauses";
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
  os << "\", \n";
}
