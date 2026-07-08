// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_adblock/adblock_service_impl.h"

#include <string>
#include <utility>

#include "base/functional/bind.h"
#include "base/task/sequenced_task_runner.h"
#include "base/task/thread_pool.h"
#include "components/phaw_adblock/filter_list_store.h"
#include "components/phaw_adblock/rust_adblock_engine.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace phaw {
namespace {
constexpr char kEnabledPref[] = "phaw.adblock.enabled";
}  // namespace

// static
void AdblockServiceImpl::RegisterProfilePrefs(PrefRegistrySimple* registry) {
  registry->RegisterBooleanPref(kEnabledPref, /*default_value=*/true);
}

AdblockServiceImpl::AdblockServiceImpl(
    PrefService* prefs,
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory)
    : prefs_(prefs),
      enabled_(prefs->GetBoolean(kEnabledPref)),
      engine_(std::make_unique<RustAdblockEngine>()),
      engine_task_runner_(base::ThreadPool::CreateSequencedTaskRunner(
          {base::TaskPriority::BEST_EFFORT})) {
  list_store_ = std::make_unique<FilterListStore>(
      std::move(url_loader_factory),
      base::BindRepeating(&AdblockServiceImpl::OnRulesReady,
                          weak_factory_.GetWeakPtr()));
  if (enabled_) {
    list_store_->Start();
  }
}

AdblockServiceImpl::~AdblockServiceImpl() = default;

void AdblockServiceImpl::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void AdblockServiceImpl::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

bool AdblockServiceImpl::IsEnabled() const {
  return enabled_;
}

void AdblockServiceImpl::SetEnabled(bool enabled) {
  if (enabled_ == enabled) {
    return;
  }
  enabled_ = enabled;
  prefs_->SetBoolean(kEnabledPref, enabled);
  if (enabled_) {
    list_store_->Start();
  }
}

void AdblockServiceImpl::AllowSite(const url::Origin& origin) {
  // Per-site allowance. (Persisting these across sessions is a small follow-up;
  // kept in-memory here.)
  site_allowlist_.insert(net::SchemefulSite(origin));
}

const AdblockEngine* AdblockServiceImpl::engine() const {
  return enabled_ ? engine_.get() : nullptr;
}

bool AdblockServiceImpl::IsSiteAllowed(const url::Origin& top_frame) const {
  return site_allowlist_.find(net::SchemefulSite(top_frame)) !=
         site_allowlist_.end();
}

void AdblockServiceImpl::ReportBlocked(const GURL& url, int tab_id) {
  ++session_blocked_;
  for (Observer& observer : observers_) {
    observer.OnResourceBlocked(url, tab_id);
    observer.OnCountersChanged(session_blocked_);
  }
}

base::WeakPtr<AdblockServiceImpl> AdblockServiceImpl::GetWeakPtr() {
  return weak_factory_.GetWeakPtr();
}

void AdblockServiceImpl::OnRulesReady(const std::string& combined_rules) {
  // Building the engine parses the whole rule set — do it off the UI thread.
  // RustAdblockEngine::SetRules is internally locked, so an in-flight Match on
  // the network sequence stays safe during the swap.
  engine_task_runner_->PostTask(
      FROM_HERE, base::BindOnce(&RustAdblockEngine::SetRules,
                                base::Unretained(engine_.get()), combined_rules));
}

}  // namespace phaw
