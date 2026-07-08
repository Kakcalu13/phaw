// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_tabs/collapse_controller_impl.h"

#include <utility>

#include "components/phaw_tabs/collapse_state_store.h"
#include "components/phaw_tabs/tab_hibernator.h"
#include "content/public/browser/web_contents.h"

namespace phaw {

CollapseControllerImpl::CollapseControllerImpl(
    TabStripModel* tab_strip_model,
    std::unique_ptr<TabHibernator> hibernator,
    CollapseStateStore* state_store)
    : tab_strip_model_(tab_strip_model),
      hibernator_(std::move(hibernator)),
      state_store_(state_store) {}

CollapseControllerImpl::~CollapseControllerImpl() = default;

void CollapseControllerImpl::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void CollapseControllerImpl::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

void CollapseControllerImpl::ToggleCollapse(
    const tab_groups::TabGroupId& group) {
  const bool will_collapse = collapsed_.find(group) == collapsed_.end();

  // confirm-in-tree: reflect the visual collapse in the tab-strip group model
  // (TabGroupModel::SetVisualData / SetTabGroupCollapsed) on the pinned milestone.

  if (will_collapse) {
    const int hibernated = hibernate_on_collapse_ ? HibernateGroup(group) : 0;
    collapsed_.insert(group);
    if (state_store_) {
      state_store_->SetCollapsed(group, true);
    }
    for (Observer& observer : observers_) {
      observer.OnGroupCollapsed(group, hibernated);
    }
  } else {
    WakeGroup(group);
    collapsed_.erase(group);
    if (state_store_) {
      state_store_->SetCollapsed(group, false);
    }
    for (Observer& observer : observers_) {
      observer.OnGroupExpanded(group);
    }
  }
}

bool CollapseControllerImpl::IsCollapsed(
    const tab_groups::TabGroupId& group) const {
  return collapsed_.find(group) != collapsed_.end();
}

void CollapseControllerImpl::SetHibernateOnCollapse(bool enabled) {
  hibernate_on_collapse_ = enabled;
}

int CollapseControllerImpl::HibernateGroup(
    const tab_groups::TabGroupId& group) {
  int hibernated = 0;
  for (content::WebContents* tab : GetTabsInGroup(group)) {
    if (hibernator_->CanHibernate(tab)) {
      hibernator_->Hibernate(tab);
      ++hibernated;
    }
  }
  return hibernated;
}

void CollapseControllerImpl::WakeGroup(const tab_groups::TabGroupId& group) {
  for (content::WebContents* tab : GetTabsInGroup(group)) {
    hibernator_->Wake(tab);
  }
}

std::vector<content::WebContents*> CollapseControllerImpl::GetTabsInGroup(
    const tab_groups::TabGroupId& group) const {
  // confirm-in-tree: enumerate a group's members via TabStripModel's group model
  // (group_model()->GetTabGroup(group)->ListTabs()) and GetWebContentsAt(index).
  return {};
}

}  // namespace phaw
