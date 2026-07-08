// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_login_cache/auth_exception_store.h"

#include <string>
#include <utility>

#include "base/values.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "url/origin.h"

namespace phaw {
namespace {
// Profile pref: list of serialized schemeful sites the user allows as 3p logins.
constexpr char kAuthExceptionsPref[] = "phaw.login_cache.auth_exceptions";
}  // namespace

// static
void AuthExceptionStore::RegisterProfilePrefs(PrefRegistrySimple* registry) {
  registry->RegisterListPref(kAuthExceptionsPref);
}

AuthExceptionStore::AuthExceptionStore(PrefService* prefs) : prefs_(prefs) {
  Load();
}

AuthExceptionStore::~AuthExceptionStore() = default;

void AuthExceptionStore::Add(const url::Origin& site) {
  if (sites_.insert(net::SchemefulSite(site)).second) {
    Save();
  }
}

bool AuthExceptionStore::Contains(const url::Origin& site) const {
  return sites_.find(net::SchemefulSite(site)) != sites_.end();
}

void AuthExceptionStore::Load() {
  sites_.clear();
  for (const base::Value& value : prefs_->GetList(kAuthExceptionsPref)) {
    const std::string* serialized = value.GetIfString();
    if (!serialized) {
      continue;
    }
    // SchemefulSite::Deserialize returns an opaque site on malformed input;
    // skip those rather than store a useless entry.
    net::SchemefulSite site = net::SchemefulSite::Deserialize(*serialized);
    if (!site.opaque()) {
      sites_.insert(std::move(site));
    }
  }
}

void AuthExceptionStore::Save() {
  base::Value::List list;
  for (const net::SchemefulSite& site : sites_) {
    list.Append(site.Serialize());
  }
  prefs_->SetList(kAuthExceptionsPref, std::move(list));
}

}  // namespace phaw
