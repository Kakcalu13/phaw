// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_ADBLOCK_RUST_ADBLOCK_ENGINE_H_
#define COMPONENTS_PHAW_ADBLOCK_RUST_ADBLOCK_ENGINE_H_

#include <string>
#include <vector>

#include "base/synchronization/lock.h"
#include "base/thread_annotations.h"
#include "components/phaw_adblock/adblock_engine.h"

// Opaque handle from adblock-rust-ffi. confirm-in-tree: the real type name from
// third_party/adblock_rust_ffi (Brave exposes `struct C_Engine`).
extern "C" {
struct C_Engine;
}

namespace phaw {

// AdblockEngine backed by adblock-rust via its C FFI. Built from concatenated
// filter-list text; Match() is thread-safe. A lock guards the swap when the rule
// set is rebuilt so in-flight queries never see a half-updated engine.
class RustAdblockEngine : public AdblockEngine {
 public:
  RustAdblockEngine();
  ~RustAdblockEngine() override;

  RustAdblockEngine(const RustAdblockEngine&) = delete;
  RustAdblockEngine& operator=(const RustAdblockEngine&) = delete;

  // (Re)builds the underlying engine from |rules| (all lists concatenated).
  // Called off the query path (e.g. after a list update).
  void SetRules(const std::string& rules);

  // AdblockEngine:
  AdblockMatch Match(const GURL& url,
                     const url::Origin& source,
                     const std::string& resource_type) const override;
  std::vector<std::string> CosmeticSelectorsFor(
      const GURL& url) const override;

 private:
  mutable base::Lock lock_;
  // Plain pointer: this is an FFI-owned opaque handle, freed via engine_destroy
  // in the destructor — not a Chromium heap object, so not a raw_ptr.
  C_Engine* engine_ GUARDED_BY(lock_) = nullptr;
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_ADBLOCK_RUST_ADBLOCK_ENGINE_H_
