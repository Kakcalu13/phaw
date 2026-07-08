// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_ADBLOCK_ADBLOCK_SERVICE_FACTORY_H_
#define COMPONENTS_PHAW_ADBLOCK_ADBLOCK_SERVICE_FACTORY_H_

#include <memory>

#include "base/no_destructor.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

class KeyedService;

namespace content {
class BrowserContext;
}  // namespace content

namespace phaw {

class AdblockService;

// Builds one AdblockService per browser context. Callers get the abstract
// interface; the concrete impl stays private to this component (rule C1).
class AdblockServiceFactory : public BrowserContextKeyedServiceFactory {
 public:
  static AdblockService* GetForBrowserContext(content::BrowserContext* context);
  static AdblockServiceFactory* GetInstance();

  AdblockServiceFactory(const AdblockServiceFactory&) = delete;
  AdblockServiceFactory& operator=(const AdblockServiceFactory&) = delete;

 private:
  friend base::NoDestructor<AdblockServiceFactory>;

  AdblockServiceFactory();
  ~AdblockServiceFactory() override;

  // BrowserContextKeyedServiceFactory:
  // confirm-in-tree: return type (std::unique_ptr<KeyedService> vs KeyedService*)
  // on the pinned milestone.
  std::unique_ptr<KeyedService> BuildServiceInstanceForBrowserContext(
      content::BrowserContext* context) const override;
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_ADBLOCK_ADBLOCK_SERVICE_FACTORY_H_
