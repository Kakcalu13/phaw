// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_adblock/cosmetic_injector.h"

#include "content/public/renderer/render_frame.h"
#include "third_party/blink/public/platform/web_string.h"
#include "third_party/blink/public/web/web_document.h"
#include "third_party/blink/public/web/web_local_frame.h"

namespace phaw {

CosmeticInjector::CosmeticInjector(content::RenderFrame* render_frame)
    : content::RenderFrameObserver(render_frame) {}

CosmeticInjector::~CosmeticInjector() = default;

void CosmeticInjector::DidCreateNewDocument() {
  // confirm-in-tree: request the cosmetic selectors + scriptlets for this
  // document's URL from the browser via the phaw_adblock mojo interface, then
  // ApplyHidingRules(selectors) and execute the scriptlets. Doing this at
  // document creation — before page scripts run — is what defeats adblock
  // detection (req 3).
}

void CosmeticInjector::OnDestruct() {
  delete this;
}

void CosmeticInjector::ApplyHidingRules(
    const std::vector<std::string>& selectors) {
  if (selectors.empty()) {
    return;
  }
  std::string css;
  for (const std::string& selector : selectors) {
    css += selector + "{display:none !important;}\n";
  }
  blink::WebLocalFrame* frame = render_frame()->GetWebFrame();
  if (frame) {
    // confirm-in-tree: WebDocument::InsertStyleSheet signature on the pinned tree.
    frame->GetDocument().InsertStyleSheet(blink::WebString::FromUTF8(css));
  }
}

}  // namespace phaw
