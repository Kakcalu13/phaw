// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_CAPTIONS_CAPTION_SERVICE_FACTORY_H_
#define COMPONENTS_PHAW_CAPTIONS_CAPTION_SERVICE_FACTORY_H_

#include <memory>

#include "base/no_destructor.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

class KeyedService;

namespace content {
class BrowserContext;
}  // namespace content

namespace phaw {

class CaptionService;

// Builds one CaptionService per browser context, wired to the SODA backend.
class CaptionServiceFactory : public BrowserContextKeyedServiceFactory {
 public:
  static CaptionService* GetForBrowserContext(content::BrowserContext* context);
  static CaptionServiceFactory* GetInstance();

  CaptionServiceFactory(const CaptionServiceFactory&) = delete;
  CaptionServiceFactory& operator=(const CaptionServiceFactory&) = delete;

 private:
  friend base::NoDestructor<CaptionServiceFactory>;

  CaptionServiceFactory();
  ~CaptionServiceFactory() override;

  // BrowserContextKeyedServiceFactory:
  std::unique_ptr<KeyedService> BuildServiceInstanceForBrowserContext(
      content::BrowserContext* context) const override;
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_CAPTIONS_CAPTION_SERVICE_FACTORY_H_
