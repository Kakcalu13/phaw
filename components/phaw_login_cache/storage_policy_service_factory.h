// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_LOGIN_CACHE_STORAGE_POLICY_SERVICE_FACTORY_H_
#define COMPONENTS_PHAW_LOGIN_CACHE_STORAGE_POLICY_SERVICE_FACTORY_H_

#include <memory>

#include "base/no_destructor.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

class KeyedService;

namespace content {
class BrowserContext;
}  // namespace content

namespace phaw {

class StoragePolicyService;

// Builds one StoragePolicyService per browser context.
class StoragePolicyServiceFactory : public BrowserContextKeyedServiceFactory {
 public:
  static StoragePolicyService* GetForBrowserContext(
      content::BrowserContext* context);
  static StoragePolicyServiceFactory* GetInstance();

  StoragePolicyServiceFactory(const StoragePolicyServiceFactory&) = delete;
  StoragePolicyServiceFactory& operator=(const StoragePolicyServiceFactory&) =
      delete;

 private:
  friend base::NoDestructor<StoragePolicyServiceFactory>;

  StoragePolicyServiceFactory();
  ~StoragePolicyServiceFactory() override;

  // BrowserContextKeyedServiceFactory:
  std::unique_ptr<KeyedService> BuildServiceInstanceForBrowserContext(
      content::BrowserContext* context) const override;
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_LOGIN_CACHE_STORAGE_POLICY_SERVICE_FACTORY_H_
