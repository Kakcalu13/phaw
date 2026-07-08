// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_ADBLOCK_FILTER_LIST_STORE_H_
#define COMPONENTS_PHAW_ADBLOCK_FILTER_LIST_STORE_H_

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "base/functional/callback.h"
#include "base/memory/scoped_refptr.h"
#include "base/timer/timer.h"
#include "url/gurl.h"

namespace network {
class SharedURLLoaderFactory;
class SimpleURLLoader;
}  // namespace network

namespace phaw {

// Fetches and periodically refreshes the filter lists (EasyList, EasyPrivacy,
// uBlock, anti-adblock — req 3 is an arms race, so refresh matters), concatenates
// them, and hands the combined rules to the engine via |on_rules_ready|.
class FilterListStore {
 public:
  using RulesReadyCallback =
      base::RepeatingCallback<void(const std::string& combined_rules)>;

  FilterListStore(
      scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
      RulesReadyCallback on_rules_ready);
  ~FilterListStore();

  FilterListStore(const FilterListStore&) = delete;
  FilterListStore& operator=(const FilterListStore&) = delete;

  // Fetches all lists now and schedules periodic refresh.
  void Start();

 private:
  void FetchAll();
  void FetchNext();
  void OnDownloaded(std::unique_ptr<std::string> body);

  scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory_;
  RulesReadyCallback on_rules_ready_;
  const std::vector<GURL> list_urls_;
  size_t next_index_ = 0;
  std::string accumulated_;
  std::unique_ptr<network::SimpleURLLoader> loader_;
  base::RepeatingTimer refresh_timer_;
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_ADBLOCK_FILTER_LIST_STORE_H_
