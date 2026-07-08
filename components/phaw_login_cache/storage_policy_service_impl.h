// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_LOGIN_CACHE_STORAGE_POLICY_SERVICE_IMPL_H_
#define COMPONENTS_PHAW_LOGIN_CACHE_STORAGE_POLICY_SERVICE_IMPL_H_

#include <memory>

#include "base/observer_list.h"
#include "components/phaw_login_cache/public/storage_policy_service.h"

class PrefRegistrySimple;
class PrefService;

namespace phaw {

class AuthExceptionStore;

// Concrete StoragePolicyService (req 5). Coordinates only: it resolves the
// per-site exception, defers the actual first-party/third-party decision to the
// stateless LoginAwareRule, and fans block events out to observers. Private to
// //components/phaw_login_cache.
class StoragePolicyServiceImpl : public StoragePolicyService {
 public:
  explicit StoragePolicyServiceImpl(PrefService* prefs);
  ~StoragePolicyServiceImpl() override;

  static void RegisterProfilePrefs(PrefRegistrySimple* registry);

  // StoragePolicyService:
  void AddObserver(Observer* observer) override;
  void RemoveObserver(Observer* observer) override;
  Decision Decide(const url::Origin& requestor,
                  const url::Origin& top_frame,
                  StorageKind kind) const override;
  void AllowLoginException(const url::Origin& site) override;

  // Called by this component's enforcement seams (cookie delegate, throttle)
  // after a third-party access is denied/partitioned, to update the UI counter.
  void NotifyThirdPartyBlocked(const url::Origin& tracker,
                               const url::Origin& top_frame);

 private:
  std::unique_ptr<AuthExceptionStore> exceptions_;
  base::ObserverList<Observer> observers_;
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_LOGIN_CACHE_STORAGE_POLICY_SERVICE_IMPL_H_
