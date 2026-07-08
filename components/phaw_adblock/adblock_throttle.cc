// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_adblock/adblock_throttle.h"

#include <string>
#include <utility>

#include "components/phaw_adblock/adblock_engine.h"
#include "net/base/net_errors.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/mojom/fetch_api.mojom-shared.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace phaw {
namespace {

// Maps Chromium's request destination to the resource-type string the filter
// engine expects. confirm-in-tree: enum members on the pinned milestone.
std::string ResourceTypeString(network::mojom::RequestDestination destination) {
  switch (destination) {
    case network::mojom::RequestDestination::kScript:
      return "script";
    case network::mojom::RequestDestination::kImage:
      return "image";
    case network::mojom::RequestDestination::kStyle:
      return "stylesheet";
    case network::mojom::RequestDestination::kFrame:
    case network::mojom::RequestDestination::kIframe:
      return "sub_frame";
    case network::mojom::RequestDestination::kFont:
      return "font";
    default:
      return "other";
  }
}

}  // namespace

AdblockThrottle::AdblockThrottle(const AdblockEngine* engine,
                                 int tab_id,
                                 ReportBlockedCallback report_blocked)
    : engine_(engine),
      tab_id_(tab_id),
      report_blocked_(std::move(report_blocked)) {}

AdblockThrottle::~AdblockThrottle() = default;

void AdblockThrottle::WillStartRequest(network::ResourceRequest* request,
                                       bool* defer) {
  if (!engine_) {
    return;
  }
  const url::Origin source =
      request->request_initiator.value_or(url::Origin::Create(request->url));
  const AdblockMatch match = engine_->Match(
      request->url, source, ResourceTypeString(request->destination));
  if (match.blocked) {
    if (report_blocked_) {
      report_blocked_.Run(request->url, tab_id_);
    }
    delegate_->CancelWithError(net::ERR_BLOCKED_BY_CLIENT);
  }
}

}  // namespace phaw
