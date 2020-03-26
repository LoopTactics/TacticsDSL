#ifndef MATCHERS_H
#define MATCHERS_H

#include "tree.h"
#include "tree_views.h"
#include <map>

namespace details {

enum OPCODE {
  Add = '+',
  Mul = '*',
};

// Assigned match to a placeholder/ArrayPlaceholder.
class AssignedMatch {
public:
  explicit AssignedMatch(std::string value) : match_(value) {}
  explicit operator bool() const { return match_ == "nullptr"; }
  bool operator!=(std::string value) const { return value != match_; }
  void operator=(std::string value) { match_ = value; }
  std::string get() const { return match_; }

private:
  std::string match_;
};

/// The matching context. It keeps track of what a given placeholder has already
/// matched. It has a "global" view of what is going on. Each placeholder when
/// instantiated register itself to the context passing its unique id. During
/// the matching, a placeholder asks the context if it can match the underneath
/// Value (i.e., not already assigned to another placeholder).
class MatchingContext {
public:
  MatchingContext() = default;
  void registerPlaceholder(size_t placeholderId);
  bool assignToPlaceholder(std::string val, size_t placeholderId);
  bool getValueForId(size_t placeholderId, std::string &value) const;
  void dump() const;

private:
  std::map<size_t, AssignedMatch> placeholderMap_;
};

// A placeholder.
class m_Placeholder {
public:
  m_Placeholder();
  m_Placeholder(const m_Placeholder &) = default;
  void dump() const;

  AssignedMatch match_;
  size_t id_;
  static MatchingContext *&context();

private:
  static thread_local size_t nextId_;
};

// Avoid passing context around.
class AccessPatternContext {
public:
  AccessPatternContext() : matchingContext_(MatchingContext()) {
    assert(m_Placeholder::context() == nullptr &&
           "Only a single matchingContext is supported");
    m_Placeholder::context() = &matchingContext_;
  }
  ~AccessPatternContext() { m_Placeholder::context() = nullptr; }
  void dump() const { return matchingContext_.dump(); }
  std::string operator[](const m_Placeholder &pl) const;

  MatchingContext matchingContext_;
};

// binary matchers.
template <typename LHS_t, typename RHS_t, unsigned Opcode>
struct BinaryOpMatch {
  LHS_t L;
  RHS_t R;

  BinaryOpMatch(const LHS_t &lhs, const RHS_t &rhs) : L(lhs), R(rhs) {}

  bool match(lang::TreeRef t) {
    if (t->kind() != Opcode)
      return false;
    return (L.match(t->trees().at(0)) && R.match(t->trees().at(1)));
  }
};

// Terminal matcher, always returns true.
struct AnyValueMatcher {
  bool match(lang::TreeRef t) const { return true; }
};

// A placeholder for Array.
class StructuredArrayPlaceholder : public m_Placeholder {
public:
  StructuredArrayPlaceholder() : placeholders_({}){};
  StructuredArrayPlaceholder operator()(std::vector<m_Placeholder> indexings) {
    this->placeholders_.clear();
    this->placeholders_ = indexings;
    return *this;
  }
  size_t size() const { return placeholders_.size(); }

public:
  std::vector<m_Placeholder> placeholders_;
};

// An access.
class OpAccessMatch {
public:
  OpAccessMatch() = delete;
  OpAccessMatch(StructuredArrayPlaceholder array) : arrayPlaceholder_(array) {}
  bool match(lang::TreeRef t);

  StructuredArrayPlaceholder arrayPlaceholder_;
};

} // end namespace details

namespace matchers {

using m_Placeholder = details::m_Placeholder;
using m_ArrayPlaceholder = details::StructuredArrayPlaceholder;
using m_Access = details::OpAccessMatch;
using m_ctx = details::AccessPatternContext;

template <typename LHS, typename RHS>
inline details::BinaryOpMatch<LHS, RHS, details::OPCODE::Add>
m_Add(const LHS &L, const RHS &R) {
  return details::BinaryOpMatch<LHS, RHS, details::OPCODE::Add>(L, R);
}
template <typename LHS, typename RHS>
inline details::BinaryOpMatch<LHS, RHS, details::OPCODE::Mul>
m_Mul(const LHS &L, const RHS &R) {
  return details::BinaryOpMatch<LHS, RHS, details::OPCODE::Mul>(L, R);
}
inline details::AnyValueMatcher m_Any() { return details::AnyValueMatcher(); }

} // end namespace matchers

#endif
