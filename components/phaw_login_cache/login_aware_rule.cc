// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_login_cache/login_aware_rule.h"

#include "net/base/schemeful_site.h"
#include "url/origin.h"

namespace phaw {

// static
StoragePolicyService::Decision LoginAwareRule::Apply(
    const url::Origin& requestor,
    const url::Origin& top_frame,
    StorageKind kind) {
  // First-party: same schemeful site (eTLD+1) as the page the user is on. This
  // is their own session with the site they chose to visit — i.e. their login,
  // and that site's own cache. Always allowed. Subdomains count as same-site.
  if (net::SchemefulSite(requestor) == net::SchemefulSite(top_frame)) {
    return Decision::kAllow;
  }

  // From here down the access is third-party.
  switch (kind) {
    case StorageKind::kCookie:
      // Third-party cookies are the primary cross-site identifier — the "follow
      // you everywhere" vector. Block outright.
      return Decision::kBlock;

    case StorageKind::kHttpCache:
    case StorageKind::kLocalStorage:
    case StorageKind::kIndexedDb:
      // Chromium already partitions these by Network Isolation Key, so a third
      // party cannot read them across sites. We keep that on and additionally
      // force session lifetime. We deliberately do NOT hard-block third-party
      // cache: that would only re-download shared CDN subresources with no
      // privacy gain over partitioning.
      return Decision::kPartitionEphemeral;
  }

  // Unreachable; deny-by-partition is the safe default.
  return Decision::kPartitionEphemeral;
}

}  // namespace phaw
