#include "matchers.h"
#include <iostream>

using namespace details;

thread_local size_t m_Placeholder::nextId_ = 0;

MatchingContext *&m_Placeholder::context() {
  thread_local MatchingContext *context = nullptr;
  return context;
}

void MatchingContext::registerPlaceholder(size_t placeholderId) {
  placeholderMap_.insert({placeholderId, AssignedMatch("nullptr")});
}

bool MatchingContext::assignToPlaceholder(std::string val,
                                          size_t placeholderId) {
  auto it = placeholderMap_.find(placeholderId);
  assert(it != placeholderMap_.end() && "placeholder not registered");

  if (it->second) {
    it->second = val;
    return true;
  }
  // Placeholder with id "placeholderId" assigned to a
  // given value "val". If we want to assing another value
  // to the same id "placeholderId", the match must exit false.
  if (it->second != val)
    return false;
  return true;
}

void MatchingContext::dump() const {
  std::cout << "dumping current context..\n";
  for (const auto &it : placeholderMap_) {
    std::cout << "first: " << it.first << "\n";
    std::cout << "second: " << it.second.get() << "\n";
    std::cout << "---\n";
  }
}

std::string AccessPatternContext::operator[](const m_Placeholder &pl) const {
  std::string value = "nullptr";
  auto result = matchingContext_.getValueForId(pl.id_, value);
  if (!result)
    assert(0 && "placeholder not found");
  return value;
}

bool MatchingContext::getValueForId(size_t placeholderId,
                                    std::string &value) const {
  auto it = placeholderMap_.find(placeholderId);
  if (it == placeholderMap_.end())
    return false;
  value = it->second.get();
  return true;
}

m_Placeholder::m_Placeholder()
    : match_(AssignedMatch("nullptr")), id_(nextId_++) {
  assert(context() != nullptr && "expect initialized context");
  auto ctx = context();
  ctx->registerPlaceholder(id_);
}

void m_Placeholder::dump() const {
  std::cout << "dumping placeholder..\n";
  std::cout << id_;
}

bool OpAccessMatch::match(lang::TreeRef t) {
  using namespace lang;
  assert(arrayPlaceholder_.size() && "must be non zero");
  if (t->kind() != TK_APPLY)
    return false;
  auto apply = Apply(t);
  auto args = apply.arguments();
  if (args.size() != arrayPlaceholder_.size())
    return false;
  auto ctx = arrayPlaceholder_.placeholders_[0].context();
  for (size_t i = 0; i < args.size(); i++) {
    if (args[i]->kind() != TK_IDENT)
      return false;
    auto operandAtPos = Ident(args[i]).name();
    auto isValidAssign = ctx->assignToPlaceholder(
        operandAtPos, arrayPlaceholder_.placeholders_[i].id_);
    if (!isValidAssign)
      return false;
  }
  auto arrayName = Ident(apply.name()).name();
  auto isValidAssign =
      ctx->assignToPlaceholder(arrayName, arrayPlaceholder_.id_);
  if (!isValidAssign)
    return false;
  return true;
}
