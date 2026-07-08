// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_ADBLOCK_ADBLOCK_ENGINE_H_
#define COMPONENTS_PHAW_ADBLOCK_ADBLOCK_ENGINE_H_

#include <string>
#include <vector>

class GURL;

namespace url {
class Origin;
}  // namespace url

namespace phaw {

// Result of matching one request against the filter rules.
struct AdblockMatch {
  bool blocked = false;       // a blocking rule matched and no exception overrode it
  bool matched_exception = false;  // an allowlist (@@) rule matched
};

// Internal engine interface. Collaborators (the throttle) depend on THIS, not on
// the concrete Rust-backed implementation (docs/architecture.md, rule D3).
//
// Implementations must be safe to query from the network sequence: Match() and
// CosmeticSelectorsFor() are const and must not mutate shared state. Rebuilding
// the rule set is a separate, non-const operation done off the query path.
class AdblockEngine {
 public:
  virtual ~AdblockEngine() = default;

  // Does |url| (a subresource requested by document at |source|, of network
  // |resource_type| e.g. "script", "image") get blocked?
  virtual AdblockMatch Match(const GURL& url,
                             const url::Origin& source,
                             const std::string& resource_type) const = 0;

  // Element-hiding CSS selectors to apply to |url|'s document (cosmetic filtering).
  virtual std::vector<std::string> CosmeticSelectorsFor(
      const GURL& url) const = 0;
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_ADBLOCK_ADBLOCK_ENGINE_H_
