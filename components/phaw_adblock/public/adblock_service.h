// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_ADBLOCK_PUBLIC_ADBLOCK_SERVICE_H_
#define COMPONENTS_PHAW_ADBLOCK_PUBLIC_ADBLOCK_SERVICE_H_

#include "base/observer_list_types.h"
#include "components/keyed_service/core/keyed_service.h"

class GURL;

namespace url {
class Origin;
}  // namespace url

namespace phaw {

// Per-profile ad/tracker blocking service (reqs 2 & 3).
//
// This is the public, abstract interface and the ONLY header outside this
// component that other layers may include (docs/architecture.md, rule C1). The
// concrete AdblockServiceImpl and every collaborator (engine, throttle, list
// store, cosmetic injector) are private to //components/phaw_adblock. Callers
// hold an AdblockService*; they never see the implementation.
class AdblockService : public KeyedService {
 public:
  // Observes block activity for the UI (block counter, shields panel).
  // Notified on the UI thread.
  class Observer : public base::CheckedObserver {
   public:
    // A request to |url| was blocked in the tab identified by |tab_id|.
    virtual void OnResourceBlocked(const GURL& url, int tab_id) {}
    // The running per-session block total changed.
    virtual void OnCountersChanged(int session_total) {}

   protected:
    ~Observer() override = default;
  };

  AdblockService(const AdblockService&) = delete;
  AdblockService& operator=(const AdblockService&) = delete;
  ~AdblockService() override = default;

  virtual void AddObserver(Observer* observer) = 0;
  virtual void RemoveObserver(Observer* observer) = 0;

  // Global on/off (req 2). Persisted per profile.
  virtual bool IsEnabled() const = 0;
  virtual void SetEnabled(bool enabled) = 0;

  // Per-site allowance: stop blocking on |origin| ("allow ads on this site").
  virtual void AllowSite(const url::Origin& origin) = 0;

 protected:
  AdblockService() = default;
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_ADBLOCK_PUBLIC_ADBLOCK_SERVICE_H_
