// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_captions/soda_caption_backend.h"

#include <utility>

#include "base/files/file_path.h"
#include "base/functional/bind.h"
#include "components/phaw_captions/language_pack_manager.h"

namespace phaw {

SodaCaptionBackend::SodaCaptionBackend()
    : language_packs_(std::make_unique<LanguagePackManager>()) {}

SodaCaptionBackend::~SodaCaptionBackend() = default;

void SodaCaptionBackend::Install(const std::string& lang,
                                 base::OnceCallback<void(bool ready)> on_ready) {
  // confirm-in-tree: kick off the SODA binary install via
  // speech::SodaInstaller::GetInstance()->InstallSoda(prefs), install the
  // language pack, and route both completions here.
  language_packs_->Install(
      lang, base::BindOnce(&SodaCaptionBackend::OnSodaInstalled,
                           base::Unretained(this), std::move(on_ready)));
}

void SodaCaptionBackend::OnSodaInstalled(
    base::OnceCallback<void(bool ready)> on_ready,
    bool install_ok) {
  if (!install_ok) {
    std::move(on_ready).Run(false);
    return;
  }
  // The decisive Win8.1 check: can the installed SODA binary actually load?
  // confirm-in-tree: resolve the installed SODA library path from SodaInstaller.
  const base::FilePath soda_library;  // TODO(phase-1): real path from installer
  const bool loaded = probe_.ProbeLoad(soda_library);
  std::move(on_ready).Run(loaded);
}

void SodaCaptionBackend::Start() {
  // confirm-in-tree: enable Live Caption (media::LiveCaptionController via the
  // kLiveCaptionEnabled pref) on the pinned milestone.
}

void SodaCaptionBackend::Stop() {
  // confirm-in-tree: disable Live Caption.
}

}  // namespace phaw
