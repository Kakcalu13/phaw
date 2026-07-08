// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_adblock/adblock_service_factory.h"

#include <utility>

#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/phaw_adblock/adblock_service_impl.h"
#include "components/phaw_adblock/public/adblock_service.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/storage_partition.h"

namespace phaw {

// static
AdblockService* AdblockServiceFactory::GetForBrowserContext(
    content::BrowserContext* context) {
  return static_cast<AdblockService*>(
      GetInstance()->GetServiceForBrowserContext(context, /*create=*/true));
}

// static
AdblockServiceFactory* AdblockServiceFactory::GetInstance() {
  static base::NoDestructor<AdblockServiceFactory> instance;
  return instance.get();
}

AdblockServiceFactory::AdblockServiceFactory()
    : BrowserContextKeyedServiceFactory(
          "PhawAdblockService",
          BrowserContextDependencyManager::GetInstance()) {}

AdblockServiceFactory::~AdblockServiceFactory() = default;

std::unique_ptr<KeyedService>
AdblockServiceFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  // confirm-in-tree: PrefService via user_prefs::UserPrefs::Get(context); the
  // browser-process URL loader factory via the default storage partition.
  PrefService* prefs = user_prefs::UserPrefs::Get(context);
  auto url_loader_factory = context->GetDefaultStoragePartition()
                                ->GetURLLoaderFactoryForBrowserProcess();
  return std::make_unique<AdblockServiceImpl>(prefs,
                                              std::move(url_loader_factory));
}

}  // namespace phaw
