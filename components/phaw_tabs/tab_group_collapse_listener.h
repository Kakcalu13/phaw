// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_TABS_TAB_GROUP_COLLAPSE_LISTENER_H_
#define COMPONENTS_PHAW_TABS_TAB_GROUP_COLLAPSE_LISTENER_H_

#include "base/memory/raw_ptr.h"
#include "chrome/browser/ui/tabs/tab_strip_model_observer.h"

class TabStripModel;

namespace phaw {

class CollapseController;

// Bridges the user's collapse/expand gesture on the tab strip to the controller
// (req 4). We observe Chromium's existing group events — we do not add new
// tab-strip state — and guard against re-entering when the controller itself
// updates the model.
class TabGroupCollapseListener : public TabStripModelObserver {
 public:
  TabGroupCollapseListener(TabStripModel* tab_strip_model,
                           CollapseController* controller);
  ~TabGroupCollapseListener() override;

  TabGroupCollapseListener(const TabGroupCollapseListener&) = delete;
  TabGroupCollapseListener& operator=(const TabGroupCollapseListener&) = delete;

  // TabStripModelObserver:
  void OnTabGroupChanged(const TabGroupChange& change) override;

 private:
  const raw_ptr<TabStripModel> tab_strip_model_;
  const raw_ptr<CollapseController> controller_;
  bool applying_ = false;  // re-entrancy guard
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_TABS_TAB_GROUP_COLLAPSE_LISTENER_H_
