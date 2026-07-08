// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_LOGIN_CACHE_STORAGE_ACCESS_THROTTLE_H_
#define COMPONENTS_PHAW_LOGIN_CACHE_STORAGE_ACCESS_THROTTLE_H_

#include "base/memory/raw_ptr.h"
#include "third_party/blink/public/common/loader/url_loader_throttle.h"

namespace network {
struct ResourceRequest;
}  // namespace network

namespace phaw {

class StoragePolicyServiceImpl;

// Applies the storage policy to subresource network requests (req 5): third-party
// cache is already NIK-partitioned by the network stack, so this throttle reports
// those accesses to the policy for the UI counter and cancels only requests the
// policy denies outright. One instance per request.
class StorageAccessThrottle : public blink::URLLoaderThrottle {
 public:
  explicit StorageAccessThrottle(StoragePolicyServiceImpl* policy);
  ~StorageAccessThrottle() override;

  StorageAccessThrottle(const StorageAccessThrottle&) = delete;
  StorageAccessThrottle& operator=(const StorageAccessThrottle&) = delete;

  // blink::URLLoaderThrottle:
  void WillStartRequest(network::ResourceRequest* request, bool* defer) override;

 private:
  const raw_ptr<StoragePolicyServiceImpl> policy_;
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_LOGIN_CACHE_STORAGE_ACCESS_THROTTLE_H_
