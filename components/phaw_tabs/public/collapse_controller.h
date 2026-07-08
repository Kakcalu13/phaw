// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_TABS_PUBLIC_COLLAPSE_CONTROLLER_H_
#define COMPONENTS_PHAW_TABS_PUBLIC_COLLAPSE_CONTROLLER_H_

#include "base/observer_list_types.h"

namespace tab_groups {
class TabGroupId;
}  // namespace tab_groups

namespace phaw {

// Collapses/expands tab groups and hibernates their tabs to free memory on
// low-spec Win8.1 machines (req 4).
//
// UI-scoped (one per browser window), so — unlike the other components — it is
// NOT a KeyedService. This is the pure interface; the implementation is
// chrome-layer (it touches the tab strip) and links against //chrome/browser.
class CollapseController {
 public:
  class Observer : public base::CheckedObserver {
   public:
    // |group| collapsed; |tabs_hibernated| of its tabs were discarded.
    virtual void OnGroupCollapsed(const tab_groups::TabGroupId& group,
                                  int tabs_hibernated) {}
    virtual void OnGroupExpanded(const tab_groups::TabGroupId& group) {}

   protected:
    ~Observer() override = default;
  };

  CollapseController(const CollapseController&) = delete;
  CollapseController& operator=(const CollapseController&) = delete;
  virtual ~CollapseController() = default;

  virtual void AddObserver(Observer* observer) = 0;
  virtual void RemoveObserver(Observer* observer) = 0;

  // Toggle collapse of |group|. On collapse, eligible tabs are hibernated
  // (active/audible/form-holding tabs are exempt); on expand they restore.
  virtual void ToggleCollapse(const tab_groups::TabGroupId& group) = 0;
  virtual bool IsCollapsed(const tab_groups::TabGroupId& group) const = 0;

  // User setting: discard tab renderers when their group collapses. When false,
  // groups collapse visually only (no hibernation).
  virtual void SetHibernateOnCollapse(bool enabled) = 0;

 protected:
  CollapseController() = default;
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_TABS_PUBLIC_COLLAPSE_CONTROLLER_H_
