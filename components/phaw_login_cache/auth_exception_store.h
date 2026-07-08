// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_LOGIN_CACHE_AUTH_EXCEPTION_STORE_H_
#define COMPONENTS_PHAW_LOGIN_CACHE_AUTH_EXCEPTION_STORE_H_

#include <set>

#include "base/memory/raw_ptr.h"
#include "net/base/schemeful_site.h"

class PrefRegistrySimple;
class PrefService;

namespace url {
class Origin;
}  // namespace url

namespace phaw {

// Persists the user's third-party login exceptions (e.g. a federated identity
// provider they trust as a third party) as schemeful sites, backed by a profile
// pref. Private to //components/phaw_login_cache.
class AuthExceptionStore {
 public:
  explicit AuthExceptionStore(PrefService* prefs);
  ~AuthExceptionStore();

  AuthExceptionStore(const AuthExceptionStore&) = delete;
  AuthExceptionStore& operator=(const AuthExceptionStore&) = delete;

  // Registers the backing list pref. Call from the profile pref registration.
  static void RegisterProfilePrefs(PrefRegistrySimple* registry);

  void Add(const url::Origin& site);
  bool Contains(const url::Origin& site) const;

 private:
  void Load();
  void Save();

  const raw_ptr<PrefService> prefs_;
  std::set<net::SchemefulSite> sites_;
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_LOGIN_CACHE_AUTH_EXCEPTION_STORE_H_
