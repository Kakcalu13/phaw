// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_login_cache/storage_policy_service_impl.h"

#include <utility>

#include "components/phaw_login_cache/auth_exception_store.h"
#include "components/phaw_login_cache/login_aware_rule.h"
#include "url/origin.h"

namespace phaw {

StoragePolicyServiceImpl::StoragePolicyServiceImpl(PrefService* prefs)
    : exceptions_(std::make_unique<AuthExceptionStore>(prefs)) {}

StoragePolicyServiceImpl::~StoragePolicyServiceImpl() = default;

// static
void StoragePolicyServiceImpl::RegisterProfilePrefs(
    PrefRegistrySimple* registry) {
  AuthExceptionStore::RegisterProfilePrefs(registry);
}

void StoragePolicyServiceImpl::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void StoragePolicyServiceImpl::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

StoragePolicyService::Decision StoragePolicyServiceImpl::Decide(
    const url::Origin& requestor,
    const url::Origin& top_frame,
    StorageKind kind) const {
  // A user-approved third party (e.g. a federated login provider) is allowed
  // before the general rule runs.
  if (exceptions_->Contains(requestor)) {
    return Decision::kAllow;
  }
  return LoginAwareRule::Apply(requestor, top_frame, kind);
}

void StoragePolicyServiceImpl::AllowLoginException(const url::Origin& site) {
  exceptions_->Add(site);
}

void StoragePolicyServiceImpl::NotifyThirdPartyBlocked(
    const url::Origin& tracker,
    const url::Origin& top_frame) {
  for (Observer& observer : observers_) {
    observer.OnThirdPartyBlocked(tracker, top_frame);
  }
}

}  // namespace phaw
