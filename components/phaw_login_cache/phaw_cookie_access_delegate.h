// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_LOGIN_CACHE_PHAW_COOKIE_ACCESS_DELEGATE_H_
#define COMPONENTS_PHAW_LOGIN_CACHE_PHAW_COOKIE_ACCESS_DELEGATE_H_

#include "base/memory/raw_ptr.h"

namespace network::mojom {
class CookieManager;
}  // namespace network::mojom

namespace url {
class Origin;
}  // namespace url

namespace phaw {

class StoragePolicyServiceImpl;

// Translates phaw's StoragePolicy into the network service's cookie
// configuration: third-party cookies blocked by default (req 5) while
// first-party (login) cookies are untouched, plus the user's login exceptions.
//
// Named "delegate" per DESIGN.md; concretely it configures a
// network::mojom::CookieManager. confirm-in-tree: on the pinned Supermium
// milestone, verify whether third-party blocking is set via
// CookieManager::BlockThirdPartyCookies or routed through cookie settings.
class PhawCookieAccessDelegate {
 public:
  explicit PhawCookieAccessDelegate(StoragePolicyServiceImpl* policy);
  ~PhawCookieAccessDelegate();

  PhawCookieAccessDelegate(const PhawCookieAccessDelegate&) = delete;
  PhawCookieAccessDelegate& operator=(const PhawCookieAccessDelegate&) = delete;

  // Applies the block-third-party-cookies policy to |cookie_manager|. Call once
  // per network context at profile initialization.
  void ConfigureCookieManager(network::mojom::CookieManager* cookie_manager);

  // Per-access predicate for callers that need a direct answer.
  bool AllowCookieAccess(const url::Origin& requestor,
                         const url::Origin& top_frame) const;

 private:
  const raw_ptr<StoragePolicyServiceImpl> policy_;
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_LOGIN_CACHE_PHAW_COOKIE_ACCESS_DELEGATE_H_
