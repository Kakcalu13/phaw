// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_adblock/rust_adblock_engine.h"

#include <utility>

#include "base/strings/string_util.h"
#include "net/base/registry_controlled_domains/registry_controlled_domain.h"
#include "url/gurl.h"
#include "url/origin.h"

// confirm-in-tree: real header path/symbols from third_party/adblock_rust_ffi.
// The Brave FFI exposes roughly:
//   C_Engine* engine_create(const char* rules);
//   void      engine_destroy(C_Engine*);
//   bool      engine_match(C_Engine*, const char* url, const char* host,
//                          const char* tab_host, bool third_party,
//                          const char* resource_type,
//                          bool* did_match_exception, ...);
#include "third_party/adblock_rust_ffi/src/lib.h"

namespace phaw {

RustAdblockEngine::RustAdblockEngine() = default;

RustAdblockEngine::~RustAdblockEngine() {
  base::AutoLock guard(lock_);
  if (engine_) {
    engine_destroy(engine_);
    engine_ = nullptr;
  }
}

void RustAdblockEngine::SetRules(const std::string& rules) {
  C_Engine* rebuilt = engine_create(rules.c_str());
  base::AutoLock guard(lock_);
  C_Engine* old = engine_;
  engine_ = rebuilt;
  if (old) {
    engine_destroy(old);
  }
}

AdblockMatch RustAdblockEngine::Match(const GURL& url,
                                      const url::Origin& source,
                                      const std::string& resource_type) const {
  const std::string host = url.host();
  const std::string tab_host = source.host();
  const bool third_party =
      !net::registry_controlled_domains::SameDomainOrHost(
          url, source,
          net::registry_controlled_domains::INCLUDE_PRIVATE_REGISTRIES);

  bool matched_exception = false;
  base::AutoLock guard(lock_);
  if (!engine_) {
    return AdblockMatch{};
  }
  const bool blocked =
      engine_match(engine_, url.spec().c_str(), host.c_str(), tab_host.c_str(),
                   third_party, resource_type.c_str(), &matched_exception);
  return AdblockMatch{/*blocked=*/blocked && !matched_exception,
                      /*matched_exception=*/matched_exception};
}

std::vector<std::string> RustAdblockEngine::CosmeticSelectorsFor(
    const GURL& url) const {
  // confirm-in-tree: the FFI cosmetic-resources call returns a JSON blob of
  // hide selectors + style rules; parse and return the hide selectors here.
  return {};
}

}  // namespace phaw
