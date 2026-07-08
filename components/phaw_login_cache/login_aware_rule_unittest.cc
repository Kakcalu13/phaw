// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_login_cache/login_aware_rule.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace phaw {
namespace {

using StorageKind = StoragePolicyService::StorageKind;
using Decision = StoragePolicyService::Decision;

url::Origin O(const char* spec) {
  return url::Origin::Create(GURL(spec));
}

TEST(LoginAwareRuleTest, FirstPartyAlwaysAllowed) {
  const url::Origin top = O("https://example.com");
  for (StorageKind kind :
       {StorageKind::kCookie, StorageKind::kHttpCache,
        StorageKind::kLocalStorage, StorageKind::kIndexedDb}) {
    // Exact origin is first-party.
    EXPECT_EQ(Decision::kAllow,
              LoginAwareRule::Apply(O("https://example.com"), top, kind));
    // Subdomains are same-site, so also first-party (your login origin).
    EXPECT_EQ(Decision::kAllow,
              LoginAwareRule::Apply(O("https://accounts.example.com"), top,
                                    kind));
  }
}

TEST(LoginAwareRuleTest, ThirdPartyCookiesBlocked) {
  EXPECT_EQ(Decision::kBlock,
            LoginAwareRule::Apply(O("https://tracker.net"),
                                  O("https://example.com"),
                                  StorageKind::kCookie));
}

TEST(LoginAwareRuleTest, ThirdPartyCacheAndStoragePartitioned) {
  const url::Origin third = O("https://tracker.net");
  const url::Origin top = O("https://example.com");
  for (StorageKind kind : {StorageKind::kHttpCache, StorageKind::kLocalStorage,
                           StorageKind::kIndexedDb}) {
    EXPECT_EQ(Decision::kPartitionEphemeral,
              LoginAwareRule::Apply(third, top, kind));
  }
}

}  // namespace
}  // namespace phaw
