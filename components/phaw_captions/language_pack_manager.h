// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_CAPTIONS_LANGUAGE_PACK_MANAGER_H_
#define COMPONENTS_PHAW_CAPTIONS_LANGUAGE_PACK_MANAGER_H_

#include <string>

#include "base/functional/callback.h"

namespace phaw {

// Requests SODA language packs via Chromium's component updater. Thin wrapper so
// the backend doesn't talk to the installer directly.
class LanguagePackManager {
 public:
  LanguagePackManager();
  ~LanguagePackManager();

  LanguagePackManager(const LanguagePackManager&) = delete;
  LanguagePackManager& operator=(const LanguagePackManager&) = delete;

  // Ensures the |lang| (e.g. "en-US") pack is installed; |on_done| reports
  // success.
  void Install(const std::string& lang, base::OnceCallback<void(bool)> on_done);
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_CAPTIONS_LANGUAGE_PACK_MANAGER_H_
