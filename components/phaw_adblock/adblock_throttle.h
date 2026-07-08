// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_ADBLOCK_ADBLOCK_THROTTLE_H_
#define COMPONENTS_PHAW_ADBLOCK_ADBLOCK_THROTTLE_H_

#include "base/functional/callback.h"
#include "base/memory/raw_ptr.h"
#include "third_party/blink/public/common/loader/url_loader_throttle.h"

class GURL;

namespace network {
struct ResourceRequest;
}  // namespace network

namespace phaw {

class AdblockEngine;

// Per-request network seam (reqs 2, 3): consults the engine on WillStartRequest
// and cancels blocked requests. Depends on the AdblockEngine INTERFACE, never on
// the Rust implementation. The engine is owned by AdblockService and outlives all
// throttles; matching is thread-safe.
class AdblockThrottle : public blink::URLLoaderThrottle {
 public:
  // Reports a blocked |url| in tab |tab_id|; bound to post to the UI thread.
  using ReportBlockedCallback = base::RepeatingCallback<void(const GURL&, int)>;

  AdblockThrottle(const AdblockEngine* engine,
                  int tab_id,
                  ReportBlockedCallback report_blocked);
  ~AdblockThrottle() override;

  AdblockThrottle(const AdblockThrottle&) = delete;
  AdblockThrottle& operator=(const AdblockThrottle&) = delete;

  // blink::URLLoaderThrottle:
  void WillStartRequest(network::ResourceRequest* request, bool* defer) override;

 private:
  const raw_ptr<const AdblockEngine> engine_;
  const int tab_id_;
  ReportBlockedCallback report_blocked_;
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_ADBLOCK_ADBLOCK_THROTTLE_H_
