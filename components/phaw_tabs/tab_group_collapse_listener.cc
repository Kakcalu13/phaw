// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_tabs/tab_group_collapse_listener.h"

#include "base/auto_reset.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "components/phaw_tabs/public/collapse_controller.h"

namespace phaw {

TabGroupCollapseListener::TabGroupCollapseListener(
    TabStripModel* tab_strip_model,
    CollapseController* controller)
    : tab_strip_model_(tab_strip_model), controller_(controller) {
  tab_strip_model_->AddObserver(this);
}

TabGroupCollapseListener::~TabGroupCollapseListener() {
  if (tab_strip_model_) {
    tab_strip_model_->RemoveObserver(this);
  }
}

void TabGroupCollapseListener::OnTabGroupChanged(const TabGroupChange& change) {
  if (applying_) {
    return;  // our own controller-driven update; ignore.
  }
  // confirm-in-tree: detect the visual "collapsed toggled" transition from
  // |change| (kVisualsChanged) on the pinned milestone, then forward the group.
  base::AutoReset<bool> guard(&applying_, true);
  // controller_->ToggleCollapse(change.group);
}

}  // namespace phaw
