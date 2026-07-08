// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_captions/soda_compat_probe.h"

#include "components/phaw_captions/public/caption_service.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace phaw {
namespace {

TEST(SodaCompatProbeTest, LoadableBinaryIsReady) {
  EXPECT_EQ(CaptionService::Status::kReady,
            SodaCompatProbe::ClassifyLoadResult(/*loaded=*/true));
}

TEST(SodaCompatProbeTest, UnloadableBinaryIsUnsupported) {
  EXPECT_EQ(CaptionService::Status::kUnsupportedOnOS,
            SodaCompatProbe::ClassifyLoadResult(/*loaded=*/false));
}

}  // namespace
}  // namespace phaw
