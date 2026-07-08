// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_TABS_RENDERER_DISCARD_HIBERNATOR_H_
#define COMPONENTS_PHAW_TABS_RENDERER_DISCARD_HIBERNATOR_H_

#include "components/phaw_tabs/tab_hibernator.h"

namespace content {
class WebContents;
}  // namespace content

namespace phaw {

// Concrete TabHibernator that frees memory by discarding a background tab's
// renderer via Chromium's resource-coordinator lifecycle machinery — the same
// mechanism used under memory pressure, triggered here on group collapse (req 4).
class RendererDiscardHibernator : public TabHibernator {
 public:
  RendererDiscardHibernator();
  ~RendererDiscardHibernator() override;

  // The pure hibernation policy, exposed for unit testing. A tab may sleep only
  // if it is not the active tab, not playing audio, and holds no unsaved form.
  static bool ShouldHibernate(bool is_active,
                              bool is_audible,
                              bool has_unsaved_form_input);

  // TabHibernator:
  bool CanHibernate(content::WebContents* tab) const override;
  void Hibernate(content::WebContents* tab) override;
  void Wake(content::WebContents* tab) override;
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_TABS_RENDERER_DISCARD_HIBERNATOR_H_
