// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_LOGIN_CACHE_PUBLIC_STORAGE_POLICY_SERVICE_H_
#define COMPONENTS_PHAW_LOGIN_CACHE_PUBLIC_STORAGE_POLICY_SERVICE_H_

#include "base/observer_list_types.h"
#include "components/keyed_service/core/keyed_service.h"

namespace url {
class Origin;
}  // namespace url

namespace phaw {

// Decides whether a storage access is allowed, blocked, or partitioned so that
// first-party login state survives while third-party tracking does not (req 5).
//
// Public, abstract interface (docs/architecture.md, rule C1). The decision rule
// itself lives in the private LoginAwareRule; this service composes it with the
// per-site exception store and notifies observers.
class StoragePolicyService : public KeyedService {
 public:
  // The kind of storage being accessed.
  enum class StorageKind {
    kCookie,
    kHttpCache,
    kLocalStorage,
    kIndexedDb,
  };

  // The outcome of a policy decision.
  enum class Decision {
    kAllow,               // first-party (your login), or an explicit exception
    kBlock,               // third-party cookies: the cross-site identifier
    kPartitionEphemeral,  // NIK-partitioned + cleared at session end
  };

  class Observer : public base::CheckedObserver {
   public:
    // A third-party |tracker| was denied/partitioned under |top_frame|.
    virtual void OnThirdPartyBlocked(const url::Origin& tracker,
                                     const url::Origin& top_frame) {}

   protected:
    ~Observer() override = default;
  };

  StoragePolicyService(const StoragePolicyService&) = delete;
  StoragePolicyService& operator=(const StoragePolicyService&) = delete;
  ~StoragePolicyService() override = default;

  virtual void AddObserver(Observer* observer) = 0;
  virtual void RemoveObserver(Observer* observer) = 0;

  // The central rule. A pure function of (requestor, top_frame, kind). MUST be
  // safe to call off the UI thread (cookie/network sequences call it).
  virtual Decision Decide(const url::Origin& requestor,
                          const url::Origin& top_frame,
                          StorageKind kind) const = 0;

  // Opt-in override: always allow |site| even as a third party (e.g. a
  // federated-login provider the user trusts).
  virtual void AllowLoginException(const url::Origin& site) = 0;

 protected:
  StoragePolicyService() = default;
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_LOGIN_CACHE_PUBLIC_STORAGE_POLICY_SERVICE_H_
