// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_tabs/renderer_discard_hibernator.h"

#include "content/public/browser/web_contents.h"

namespace phaw {

RendererDiscardHibernator::RendererDiscardHibernator() = default;
RendererDiscardHibernator::~RendererDiscardHibernator() = default;

// static
bool RendererDiscardHibernator::ShouldHibernate(bool is_active,
                                                bool is_audible,
                                                bool has_unsaved_form_input) {
  // Never sleep the tab the user is looking at, one that's playing audio, or one
  // with unsaved input — hibernation must not lose work or interrupt the user.
  return !is_active && !is_audible && !has_unsaved_form_input;
}

bool RendererDiscardHibernator::CanHibernate(content::WebContents* tab) const {
  if (!tab) {
    return false;
  }
  // confirm-in-tree: exact accessors for active/audible/unsaved-form on the
  // pinned milestone (e.g. IsCurrentlyAudible(); active is decided by the caller
  // which only passes background tabs; unsaved-form via FormInteractionTabHelper).
  const bool is_active = false;  // controller only enumerates background tabs
  const bool is_audible = tab->IsCurrentlyAudible();
  const bool has_unsaved_form_input = false;  // TODO(phase-1): query form helper
  return ShouldHibernate(is_active, is_audible, has_unsaved_form_input);
}

void RendererDiscardHibernator::Hibernate(content::WebContents* tab) {
  if (!tab) {
    return;
  }
  // confirm-in-tree: discard via resource_coordinator::TabLifecycleUnitExternal
  // ::FromWebContents(tab)->DiscardTab(LifecycleUnitDiscardReason::PROACTIVE) on
  // the pinned milestone.
}

void RendererDiscardHibernator::Wake(content::WebContents* tab) {
  // No-op: a discarded tab reloads automatically when next activated. Present so
  // callers program against the interface symmetrically.
}

}  // namespace phaw
