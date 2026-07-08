// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_captions/caption_service_factory.h"

#include <memory>

#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/phaw_captions/caption_service_impl.h"
#include "components/phaw_captions/public/caption_service.h"
#include "components/phaw_captions/soda_caption_backend.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/browser_context.h"

namespace phaw {

// static
CaptionService* CaptionServiceFactory::GetForBrowserContext(
    content::BrowserContext* context) {
  return static_cast<CaptionService*>(
      GetInstance()->GetServiceForBrowserContext(context, /*create=*/true));
}

// static
CaptionServiceFactory* CaptionServiceFactory::GetInstance() {
  static base::NoDestructor<CaptionServiceFactory> instance;
  return instance.get();
}

CaptionServiceFactory::CaptionServiceFactory()
    : BrowserContextKeyedServiceFactory(
          "PhawCaptionService",
          BrowserContextDependencyManager::GetInstance()) {}

CaptionServiceFactory::~CaptionServiceFactory() = default;

std::unique_ptr<KeyedService>
CaptionServiceFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  return std::make_unique<CaptionServiceImpl>(
      user_prefs::UserPrefs::Get(context),
      std::make_unique<SodaCaptionBackend>());
}

}  // namespace phaw

