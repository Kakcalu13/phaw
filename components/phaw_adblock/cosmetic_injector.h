// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_ADBLOCK_COSMETIC_INJECTOR_H_
#define COMPONENTS_PHAW_ADBLOCK_COSMETIC_INJECTOR_H_

#include <string>
#include <vector>

#include "content/public/renderer/render_frame_observer.h"

namespace content {
class RenderFrame;
}  // namespace content

namespace phaw {

// Renderer-side cosmetic filtering (reqs 2, 3). Lives with each RenderFrame;
// on new-document creation it hides ad elements and runs anti-adblock scriptlets
// BEFORE page scripts run — which is what prevents blocker-detectors from firing.
// Selectors/scriptlets come from the browser over the phaw_adblock mojo channel.
class CosmeticInjector : public content::RenderFrameObserver {
 public:
  explicit CosmeticInjector(content::RenderFrame* render_frame);
  ~CosmeticInjector() override;

  CosmeticInjector(const CosmeticInjector&) = delete;
  CosmeticInjector& operator=(const CosmeticInjector&) = delete;

  // content::RenderFrameObserver:
  void DidCreateNewDocument() override;
  void OnDestruct() override;

 private:
  // Injects `selector{display:none!important}` rules for element hiding.
  void ApplyHidingRules(const std::vector<std::string>& selectors);
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_ADBLOCK_COSMETIC_INJECTOR_H_
