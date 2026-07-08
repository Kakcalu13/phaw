// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_adblock/rust_adblock_engine.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"
#include "url/origin.h"

// Exercises the real adblock-rust FFI through RustAdblockEngine, so it runs when
// the component is built in-tree with third_party/adblock_rust_ffi linked.
namespace phaw {
namespace {

TEST(RustAdblockEngineTest, BlocksMatchingThirdPartyRequest) {
  RustAdblockEngine engine;
  engine.SetRules("||ads.example.com^\n");

  const url::Origin source =
      url::Origin::Create(GURL("https://news.example.org"));

  EXPECT_TRUE(engine
                  .Match(GURL("https://ads.example.com/a.js"), source, "script")
                  .blocked);
  EXPECT_FALSE(
      engine.Match(GURL("https://news.example.org/app.js"), source, "script")
          .blocked);
}

TEST(RustAdblockEngineTest, ExceptionRuleUnblocks) {
  RustAdblockEngine engine;
  engine.SetRules("||ads.example.com^\n@@||ads.example.com/allowed^\n");

  const url::Origin source =
      url::Origin::Create(GURL("https://news.example.org"));

  const AdblockMatch match =
      engine.Match(GURL("https://ads.example.com/allowed/x.js"), source,
                   "script");
  EXPECT_FALSE(match.blocked);
  EXPECT_TRUE(match.matched_exception);
}

}  // namespace
}  // namespace phaw
