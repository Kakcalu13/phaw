// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_TABS_COLLAPSE_STATE_STORE_H_
#define COMPONENTS_PHAW_TABS_COLLAPSE_STATE_STORE_H_

#include <set>

#include "base/memory/raw_ptr.h"
#include "components/tab_groups/tab_group_id.h"

class PrefRegistrySimple;
class PrefService;

namespace phaw {

// Persists which groups are collapsed so the state survives a restart (req 4).
// confirm-in-tree: durable persistence should key on saved-tab-group GUIDs
// (SavedTabGroup), since a TabGroupId token is regenerated per session; this
// store keeps the in-session set plus a best-effort pref.
class CollapseStateStore {
 public:
  explicit CollapseStateStore(PrefService* prefs);
  ~CollapseStateStore();

  CollapseStateStore(const CollapseStateStore&) = delete;
  CollapseStateStore& operator=(const CollapseStateStore&) = delete;

  static void RegisterProfilePrefs(PrefRegistrySimple* registry);

  void SetCollapsed(const tab_groups::TabGroupId& group, bool collapsed);
  bool IsCollapsed(const tab_groups::TabGroupId& group) const;

 private:
  void Load();
  void Save();

  const raw_ptr<PrefService> prefs_;
  std::set<tab_groups::TabGroupId> collapsed_;
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_TABS_COLLAPSE_STATE_STORE_H_
