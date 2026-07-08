// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_captions/language_pack_manager.h"

#include <utility>

namespace phaw {

LanguagePackManager::LanguagePackManager() = default;
LanguagePackManager::~LanguagePackManager() = default;

void LanguagePackManager::Install(const std::string& lang,
                                  base::OnceCallback<void(bool)> on_done) {
  // confirm-in-tree: request the pack via speech::SodaInstaller::InstallLanguage
  // (or the components/soda language API) on the pinned milestone, and invoke
  // |on_done| from its completion observer.
  std::move(on_done).Run(/*success=*/false);
}

}  // namespace phaw
