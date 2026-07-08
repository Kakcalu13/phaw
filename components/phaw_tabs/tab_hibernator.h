// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_TABS_TAB_HIBERNATOR_H_
#define COMPONENTS_PHAW_TABS_TAB_HIBERNATOR_H_

namespace content {
class WebContents;
}  // namespace content

namespace phaw {

// Internal seam for putting a tab to sleep and waking it. The controller depends
// on this interface (docs/architecture.md, rule D3), so its hibernation policy is
// testable with a fake and independent of the concrete discard mechanism.
class TabHibernator {
 public:
  virtual ~TabHibernator() = default;

  // A tab is eligible only if hibernating it can't lose work or interrupt the
  // user (see RendererDiscardHibernator for the concrete policy).
  virtual bool CanHibernate(content::WebContents* tab) const = 0;

  // Discards the tab's renderer (kept in the strip; reloads lazily on activation).
  virtual void Hibernate(content::WebContents* tab) = 0;

  // Ensures the tab is live again (no-op if it was never discarded).
  virtual void Wake(content::WebContents* tab) = 0;
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_TABS_TAB_HIBERNATOR_H_
