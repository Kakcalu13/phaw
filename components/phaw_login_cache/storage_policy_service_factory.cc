// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_login_cache/storage_policy_service_factory.h"

#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/phaw_login_cache/public/storage_policy_service.h"
#include "components/phaw_login_cache/storage_policy_service_impl.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/browser_context.h"

namespace phaw {

// static
StoragePolicyService* StoragePolicyServiceFactory::GetForBrowserContext(
    content::BrowserContext* context) {
  return static_cast<StoragePolicyService*>(
      GetInstance()->GetServiceForBrowserContext(context, /*create=*/true));
}

// static
StoragePolicyServiceFactory* StoragePolicyServiceFactory::GetInstance() {
  static base::NoDestructor<StoragePolicyServiceFactory> instance;
  return instance.get();
}

StoragePolicyServiceFactory::StoragePolicyServiceFactory()
    : BrowserContextKeyedServiceFactory(
          "PhawStoragePolicyService",
          BrowserContextDependencyManager::GetInstance()) {}

StoragePolicyServiceFactory::~StoragePolicyServiceFactory() = default;

std::unique_ptr<KeyedService>
StoragePolicyServiceFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  return std::make_unique<StoragePolicyServiceImpl>(
      user_prefs::UserPrefs::Get(context));
}

}  // namespace phaw
