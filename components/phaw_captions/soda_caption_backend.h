// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_CAPTIONS_SODA_CAPTION_BACKEND_H_
#define COMPONENTS_PHAW_CAPTIONS_SODA_CAPTION_BACKEND_H_

#include <memory>
#include <string>

#include "base/functional/callback.h"
#include "components/phaw_captions/caption_backend.h"
#include "components/phaw_captions/soda_compat_probe.h"

namespace phaw {

class LanguagePackManager;

// Concrete CaptionBackend over Chromium's SODA + Live Caption. Install() drives
// the SODA/component installer and then runs SodaCompatProbe so a Win8.1 machine
// where SODA can't load is reported as unsupported rather than crashing (req 1).
class SodaCaptionBackend : public CaptionBackend {
 public:
  SodaCaptionBackend();
  ~SodaCaptionBackend() override;

  SodaCaptionBackend(const SodaCaptionBackend&) = delete;
  SodaCaptionBackend& operator=(const SodaCaptionBackend&) = delete;

  // CaptionBackend:
  void Install(const std::string& lang,
               base::OnceCallback<void(bool ready)> on_ready) override;
  void Start() override;
  void Stop() override;

 private:
  void OnSodaInstalled(base::OnceCallback<void(bool ready)> on_ready,
                       bool install_ok);

  SodaCompatProbe probe_;
  std::unique_ptr<LanguagePackManager> language_packs_;
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_CAPTIONS_SODA_CAPTION_BACKEND_H_
