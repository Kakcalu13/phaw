// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_login_cache/storage_access_throttle.h"

#include "components/phaw_login_cache/public/storage_policy_service.h"
#include "components/phaw_login_cache/storage_policy_service_impl.h"
#include "net/base/net_errors.h"
#include "services/network/public/cpp/resource_request.h"
#include "url/origin.h"

namespace phaw {

StorageAccessThrottle::StorageAccessThrottle(StoragePolicyServiceImpl* policy)
    : policy_(policy) {}

StorageAccessThrottle::~StorageAccessThrottle() = default;

void StorageAccessThrottle::WillStartRequest(network::ResourceRequest* request,
                                             bool* defer) {
  // Who is making the request (falls back to the request's own origin).
  const url::Origin requestor =
      request->request_initiator.value_or(url::Origin::Create(request->url));

  // Top-frame origin from the request's isolation info.
  // confirm-in-tree: accessor path (trusted_params->isolation_info
  // .top_frame_origin()) on the pinned milestone.
  url::Origin top_frame = requestor;
  if (request->trusted_params &&
      request->trusted_params->isolation_info.top_frame_origin()) {
    top_frame = *request->trusted_params->isolation_info.top_frame_origin();
  }

  const StoragePolicyService::Decision decision = policy_->Decide(
      requestor, top_frame, StoragePolicyService::StorageKind::kHttpCache);

  switch (decision) {
    case StoragePolicyService::Decision::kBlock:
      delegate_->CancelWithError(net::ERR_BLOCKED_BY_CLIENT);
      return;
    case StoragePolicyService::Decision::kPartitionEphemeral:
      // Cache is already partitioned by the network stack; surface it for UI.
      policy_->NotifyThirdPartyBlocked(requestor, top_frame);
      return;
    case StoragePolicyService::Decision::kAllow:
      return;
  }
}

}  // namespace phaw
