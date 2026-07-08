// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_CAPTIONS_CAPTION_BACKEND_H_
#define COMPONENTS_PHAW_CAPTIONS_CAPTION_BACKEND_H_

#include <string>

#include "base/functional/callback.h"

namespace phaw {

// Internal seam over Chromium's Live Caption / SODA machinery. The service
// depends on THIS interface (docs/architecture.md, rule D3), so its state machine
// is testable with a fake and independent of SODA.
class CaptionBackend {
 public:
  virtual ~CaptionBackend() = default;

  // Installs SODA + the |lang| language pack, then probes that it can actually
  // load on this OS. |on_ready| reports whether captions can run here (false =>
  // unsupported on this OS, e.g. Win8.1 where the SODA binary won't load).
  virtual void Install(const std::string& lang,
                       base::OnceCallback<void(bool ready)> on_ready) = 0;

  virtual void Start() = 0;
  virtual void Stop() = 0;
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_CAPTIONS_CAPTION_BACKEND_H_
