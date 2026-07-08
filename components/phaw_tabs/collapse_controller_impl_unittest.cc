// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_tabs/renderer_discard_hibernator.h"

#include "testing/gtest/include/gtest/gtest.h"

// The controller's hibernation decision reduces to this pure policy, which is
// fully unit-testable. Full controller coverage (tab-strip enumeration, observer
// fan-out, persistence) belongs in a browser_test where a real TabStripModel and
// WebContents exist.
namespace phaw {
namespace {

using H = RendererDiscardHibernator;

TEST(CollapseControllerHibernationPolicyTest, EligibleBackgroundTabSleeps) {
  EXPECT_TRUE(H::ShouldHibernate(/*is_active=*/false, /*is_audible=*/false,
                                 /*has_unsaved_form_input=*/false));
}

TEST(CollapseControllerHibernationPolicyTest, ExemptsActiveAudibleAndForms) {
  EXPECT_FALSE(H::ShouldHibernate(/*is_active=*/true, false, false));
  EXPECT_FALSE(H::ShouldHibernate(false, /*is_audible=*/true, false));
  EXPECT_FALSE(H::ShouldHibernate(false, false, /*has_unsaved_form_input=*/true));
}

}  // namespace
}  // namespace phaw
