// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_login_cache/phaw_cookie_access_delegate.h"

#include "components/phaw_login_cache/public/storage_policy_service.h"
#include "components/phaw_login_cache/storage_policy_service_impl.h"
#include "services/network/public/mojom/cookie_manager.mojom.h"
#include "url/origin.h"

namespace phaw {

PhawCookieAccessDelegate::PhawCookieAccessDelegate(
    StoragePolicyServiceImpl* policy)
    : policy_(policy) {}

PhawCookieAccessDelegate::~PhawCookieAccessDelegate() = default;

void PhawCookieAccessDelegate::ConfigureCookieManager(
    network::mojom::CookieManager* cookie_manager) {
  // Block third-party cookies globally. First-party (login/session) cookies are
  // unaffected, so the user stays signed in. User-approved exceptions are layered
  // on as content settings by the profile wiring.
  // confirm-in-tree: exact method/params on the pinned milestone.
  cookie_manager->BlockThirdPartyCookies(true);
}

bool PhawCookieAccessDelegate::AllowCookieAccess(
    const url::Origin& requestor,
    const url::Origin& top_frame) const {
  return policy_->Decide(requestor, top_frame,
                         StoragePolicyService::StorageKind::kCookie) ==
         StoragePolicyService::Decision::kAllow;
}

}  // namespace phaw
