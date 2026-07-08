// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_TABS_COLLAPSE_CONTROLLER_IMPL_H_
#define COMPONENTS_PHAW_TABS_COLLAPSE_CONTROLLER_IMPL_H_

#include <memory>
#include <set>
#include <vector>

#include "base/memory/raw_ptr.h"
#include "base/observer_list.h"
#include "components/phaw_tabs/public/collapse_controller.h"
#include "components/tab_groups/tab_group_id.h"

class TabStripModel;

namespace content {
class WebContents;
}  // namespace content

namespace phaw {

class CollapseStateStore;
class TabHibernator;

// Concrete CollapseController (req 4). Coordinates the collapse gesture with
// hibernation and persistence; the discard mechanism is injected as a
// TabHibernator so the policy is testable. Chrome-layer (owns a TabStripModel
// pointer) — see BUILD.gn note on placement.
class CollapseControllerImpl : public CollapseController {
 public:
  CollapseControllerImpl(TabStripModel* tab_strip_model,
                         std::unique_ptr<TabHibernator> hibernator,
                         CollapseStateStore* state_store);
  ~CollapseControllerImpl() override;

  CollapseControllerImpl(const CollapseControllerImpl&) = delete;
  CollapseControllerImpl& operator=(const CollapseControllerImpl&) = delete;

  // CollapseController:
  void AddObserver(Observer* observer) override;
  void RemoveObserver(Observer* observer) override;
  void ToggleCollapse(const tab_groups::TabGroupId& group) override;
  bool IsCollapsed(const tab_groups::TabGroupId& group) const override;
  void SetHibernateOnCollapse(bool enabled) override;

 private:
  // Hibernates the eligible background tabs of |group|; returns how many slept.
  int HibernateGroup(const tab_groups::TabGroupId& group);
  void WakeGroup(const tab_groups::TabGroupId& group);
  std::vector<content::WebContents*> GetTabsInGroup(
      const tab_groups::TabGroupId& group) const;

  const raw_ptr<TabStripModel> tab_strip_model_;
  const std::unique_ptr<TabHibernator> hibernator_;
  const raw_ptr<CollapseStateStore> state_store_;

  // confirm-in-tree: tab_groups::TabGroupId ordering for std::set on the pinned
  // milestone (its wrapped base::Token is ordered).
  std::set<tab_groups::TabGroupId> collapsed_;
  bool hibernate_on_collapse_ = true;
  base::ObserverList<Observer> observers_;
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_TABS_COLLAPSE_CONTROLLER_IMPL_H_
