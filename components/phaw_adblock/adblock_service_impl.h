// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_ADBLOCK_ADBLOCK_SERVICE_IMPL_H_
#define COMPONENTS_PHAW_ADBLOCK_ADBLOCK_SERVICE_IMPL_H_

#include <memory>
#include <set>

#include "base/memory/raw_ptr.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "components/phaw_adblock/public/adblock_service.h"
#include "net/base/schemeful_site.h"

class GURL;
class PrefRegistrySimple;
class PrefService;

namespace base {
class SequencedTaskRunner;
}  // namespace base

namespace network {
class SharedURLLoaderFactory;
}  // namespace network

namespace phaw {

class AdblockEngine;
class FilterListStore;
class RustAdblockEngine;

// Concrete AdblockService (reqs 2, 3). Coordinates only: owns the engine and the
// list store, tracks enabled/allowlist state, and fans block events out to
// observers. The actual matching lives in the engine; list fetching in the store.
class AdblockServiceImpl : public AdblockService {
 public:
  AdblockServiceImpl(
      PrefService* prefs,
      scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory);
  ~AdblockServiceImpl() override;

  static void RegisterProfilePrefs(PrefRegistrySimple* registry);

  // AdblockService:
  void AddObserver(Observer* observer) override;
  void RemoveObserver(Observer* observer) override;
  bool IsEnabled() const override;
  void SetEnabled(bool enabled) override;
  void AllowSite(const url::Origin& origin) override;

  // For the throttle provider (wired in patches/): the engine to query, or
  // nullptr when blocking is disabled; and the per-site allow check so allowed
  // top-frames get no throttle at all.
  const AdblockEngine* engine() const;
  bool IsSiteAllowed(const url::Origin& top_frame) const;

  // Block-report sink for throttles (posted to the UI thread).
  void ReportBlocked(const GURL& url, int tab_id);

  base::WeakPtr<AdblockServiceImpl> GetWeakPtr();

 private:
  void OnRulesReady(const std::string& combined_rules);

  const raw_ptr<PrefService> prefs_;
  bool enabled_ = true;
  int session_blocked_ = 0;
  std::set<net::SchemefulSite> site_allowlist_;

  std::unique_ptr<RustAdblockEngine> engine_;
  std::unique_ptr<FilterListStore> list_store_;
  scoped_refptr<base::SequencedTaskRunner> engine_task_runner_;
  base::ObserverList<Observer> observers_;

  base::WeakPtrFactory<AdblockServiceImpl> weak_factory_{this};
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_ADBLOCK_ADBLOCK_SERVICE_IMPL_H_
