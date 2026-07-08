// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_adblock/filter_list_store.h"

#include <utility>

#include "base/time/time.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "services/network/public/cpp/simple_url_loader.h"

namespace phaw {
namespace {

// Refresh cadence. Filter lists (esp. anti-adblock) change often; 12h is a
// reasonable balance for a background browser process.
constexpr base::TimeDelta kRefreshInterval = base::Hours(12);

// Cap a single list download so a hostile mirror can't exhaust memory.
constexpr size_t kMaxListBytes = 32u * 1024u * 1024u;  // 32 MiB

std::vector<GURL> DefaultLists() {
  return {
      GURL("https://easylist.to/easylist/easylist.txt"),
      GURL("https://easylist.to/easylist/easyprivacy.txt"),
      GURL("https://ublockorigin.github.io/uAssets/filters/filters.txt"),
      // Anti-adblock defeat lists (req 3).
      GURL("https://easylist-downloads.adblockplus.org/antiadblockfilters.txt"),
  };
}

}  // namespace

FilterListStore::FilterListStore(
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
    RulesReadyCallback on_rules_ready)
    : url_loader_factory_(std::move(url_loader_factory)),
      on_rules_ready_(std::move(on_rules_ready)),
      list_urls_(DefaultLists()) {}

FilterListStore::~FilterListStore() = default;

void FilterListStore::Start() {
  FetchAll();
  refresh_timer_.Start(FROM_HERE, kRefreshInterval, this,
                       &FilterListStore::FetchAll);
}

void FilterListStore::FetchAll() {
  next_index_ = 0;
  accumulated_.clear();
  FetchNext();
}

void FilterListStore::FetchNext() {
  if (next_index_ >= list_urls_.size()) {
    on_rules_ready_.Run(accumulated_);
    return;
  }

  auto request = std::make_unique<network::ResourceRequest>();
  request->url = list_urls_[next_index_];
  request->credentials_mode = network::mojom::CredentialsMode::kOmit;

  // confirm-in-tree: replace with a real, reviewed traffic annotation.
  loader_ = network::SimpleURLLoader::Create(std::move(request),
                                             MISSING_TRAFFIC_ANNOTATION);
  loader_->DownloadToString(
      url_loader_factory_.get(),
      base::BindOnce(&FilterListStore::OnDownloaded, base::Unretained(this)),
      kMaxListBytes);
}

void FilterListStore::OnDownloaded(std::unique_ptr<std::string> body) {
  if (body) {
    accumulated_.append(*body);
    accumulated_.push_back('\n');
  }
  // A failed list is skipped, not fatal — the others still apply.
  ++next_index_;
  FetchNext();
}

}  // namespace phaw
