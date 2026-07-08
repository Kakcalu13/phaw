// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_tabs/collapse_state_store.h"

#include <string>
#include <utility>

#include "base/token.h"
#include "base/values.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"

namespace phaw {
namespace {
constexpr char kCollapsedGroupsPref[] = "phaw.tabs.collapsed_groups";
}  // namespace

// static
void CollapseStateStore::RegisterProfilePrefs(PrefRegistrySimple* registry) {
  registry->RegisterListPref(kCollapsedGroupsPref);
}

CollapseStateStore::CollapseStateStore(PrefService* prefs) : prefs_(prefs) {
  Load();
}

CollapseStateStore::~CollapseStateStore() = default;

void CollapseStateStore::SetCollapsed(const tab_groups::TabGroupId& group,
                                      bool collapsed) {
  const bool changed =
      collapsed ? collapsed_.insert(group).second : collapsed_.erase(group) > 0;
  if (changed) {
    Save();
  }
}

bool CollapseStateStore::IsCollapsed(
    const tab_groups::TabGroupId& group) const {
  return collapsed_.find(group) != collapsed_.end();
}

void CollapseStateStore::Load() {
  collapsed_.clear();
  for (const base::Value& value : prefs_->GetList(kCollapsedGroupsPref)) {
    const std::string* serialized = value.GetIfString();
    if (!serialized) {
      continue;
    }
    // confirm-in-tree: base::Token::FromString / TabGroupId::FromRawToken names
    // on the pinned milestone.
    std::optional<base::Token> token = base::Token::FromString(*serialized);
    if (token) {
      collapsed_.insert(tab_groups::TabGroupId::FromRawToken(*token));
    }
  }
}

void CollapseStateStore::Save() {
  base::Value::List list;
  for (const tab_groups::TabGroupId& group : collapsed_) {
    list.Append(group.token().ToString());
  }
  prefs_->SetList(kCollapsedGroupsPref, std::move(list));
}

}  // namespace phaw
