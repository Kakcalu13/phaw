// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_LOGIN_CACHE_LOGIN_AWARE_RULE_H_
#define COMPONENTS_PHAW_LOGIN_CACHE_LOGIN_AWARE_RULE_H_

#include "components/phaw_login_cache/public/storage_policy_service.h"

namespace url {
class Origin;
}  // namespace url

namespace phaw {

// The pure decision rule behind StoragePolicyService (req 5). Stateless and
// side-effect-free, so it is fully unit-testable without a browser. Per-site
// exceptions are resolved by the service *before* this is consulted, so the rule
// only sees the first-party vs third-party question.
class LoginAwareRule {
 public:
  using StorageKind = StoragePolicyService::StorageKind;
  using Decision = StoragePolicyService::Decision;

  // Decide the fate of |requestor|'s |kind| access under top-level |top_frame|.
  static Decision Apply(const url::Origin& requestor,
                        const url::Origin& top_frame,
                        StorageKind kind);
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_LOGIN_CACHE_LOGIN_AWARE_RULE_H_
